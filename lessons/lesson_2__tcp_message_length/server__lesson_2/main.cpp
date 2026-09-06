#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <string_view>
#include <unistd.h>


constexpr int PORT = 8000;
constexpr std::string_view IP = "127.0.0.1";


int main()
{
    // Create an IPv4 TCP socket.
    int socket_fd_server = socket(AF_INET, SOCK_STREAM, 0);

    // Configure the address on which the server will listen.
    sockaddr_in addr_server{};
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(PORT);

    // Convert "127.0.0.1" from text into the binary format
    // expected by sockaddr_in.
    inet_pton(AF_INET, IP.data(), &addr_server.sin_addr);

    // Assign our IP address and port to the socket.
    bind(
        socket_fd_server,
        (sockaddr*)&addr_server,
        sizeof(addr_server)
    );

    // Turn the socket into a listening socket.
    // The second argument controls the connection backlog.
    listen(socket_fd_server, 1);

    spdlog::info("Waiting for client...");

    sockaddr_in addr_client{};
    socklen_t client_len = sizeof(addr_client);

    // Wait until a client connects.
    // accept() returns a NEW socket used to communicate with that client.
    // socket_fd_server remains the listening socket.
    int socket_fd_client = accept(
        socket_fd_server,
        (sockaddr*)&addr_client,
        &client_len
    );

    // Convert the client's binary IP address into a readable string.
    char client_ip[INET_ADDRSTRLEN]{};

    inet_ntop(
        AF_INET,
        &addr_client.sin_addr,
        client_ip,
        sizeof(client_ip)
    );

    // Ports in sockaddr_in are stored in network byte order.
    // Convert the client's port to the host's byte order.
    int client_port = ntohs(addr_client.sin_port);


    // Our protocol:
    //
    // [ 1 byte: message size ][ message bytes... ]
    //
    // First receive the one-byte message size.
    uint8_t message_size = 0;

    ssize_t bytes_received = recv(
        socket_fd_client,
        &message_size,
        sizeof(message_size),
        0
    );

    // recv() returns:
    //   > 0 : number of bytes received
    //   = 0 : client closed the connection
    //   < 0 : an error occurred
    if (bytes_received <= 0)
    {
        spdlog::error("Failed to receive message size");

        close(socket_fd_client);
        close(socket_fd_server);

        return 1;
    }

    spdlog::info("Incoming message size: {}", message_size);


    // Create exactly enough space for the incoming payload.
    std::string message_received;
    message_received.resize(message_size);

    size_t total_received = 0;

    // TCP is a byte stream. One recv() is NOT guaranteed to return
    // the entire message, so keep receiving until we have all bytes.
    while (total_received < message_received.size())
    {
        ssize_t received = recv(
            socket_fd_client,
            // Continue writing after the bytes we already received.
            message_received.data() + total_received,
            // Ask only for the number of bytes still missing.
            message_received.size() - total_received,
            0
        );

        if (received <= 0)
        {
            spdlog::error(
                "Connection closed while receiving message"
            );
            break;
        }

        // Remember how many bytes of the message we have so far.
        total_received += static_cast<size_t>(received);
    }

    spdlog::info(
        "Received '{}' from {}:{}",
        message_received,
        client_ip,
        client_port
    );

    // Close the connected client socket first,
    // then the server's listening socket.
    close(socket_fd_client);
    close(socket_fd_server);

    return 0;
}