#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cerrno>
#include <exception>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>


constexpr int PORT = 8000;
constexpr std::string_view IP = "127.0.0.1";

// Number of bytes used to represent the message length.
constexpr uint16_t BYTES_MESSAGE_LENGTH = 2;

static_assert(BYTES_MESSAGE_LENGTH == sizeof(uint16_t));


/**
 * @brief Reads the message length from a TCP socket.
 *
 * Reads exactly two bytes from the socket, representing an unsigned
 * 16-bit message length in network byte order (big-endian).
 *
 * Since TCP is a byte stream, multiple recv() calls may be required
 * to receive the complete length prefix.
 *
 * @param socket_fd_client File descriptor of the connected client socket.
 *
 * @return Message length in bytes, converted to host byte order.
 *
 * @throws std::runtime_error If the client disconnects before both bytes are received.
 * @throws std::system_error If recv() fails for a reason other than EINTR.
 */
uint16_t read_message_size(int socket_fd_client)
{
    // Our protocol:
    //
    // [ 2 bytes: message size ][ message bytes... ]
    //
    // The message size is stored in network byte order (big-endian).
    //
    // First receive exactly two bytes containing the message size.
    uint16_t message_size_network = 0;

    size_t total_size_received = 0;

    // TCP is a byte stream. One recv() is NOT guaranteed to return
    // both bytes, so keep receiving until we have exactly two bytes.
    while (total_size_received < BYTES_MESSAGE_LENGTH)
    {
        ssize_t received = recv(socket_fd_client,
                                reinterpret_cast<uint8_t*>(&message_size_network) + total_size_received,
                                BYTES_MESSAGE_LENGTH - total_size_received,
                                0);

        if (received < 0 && errno == EINTR)
            continue;

        if (received == 0)
            throw std::runtime_error("Client disconnected before the complete message length was received");

        if (received < 0)
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to receive message length");

        total_size_received += static_cast<size_t>(received);
    }

    // Convert the message size from network byte order (big-endian)
    // to the host's native byte order.
    uint16_t message_size = ntohs(message_size_network);

    return message_size;
}


int main()
{
    // Create an IPv4 TCP socket.
    int socket_fd_server = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd_server < 0)
    {
        spdlog::error("Failed to create server socket");
        return 1;
    }

    // Configure the address on which the server will listen.
    sockaddr_in addr_server{};
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(PORT);

    // Convert "127.0.0.1" from text into the binary format
    // expected by sockaddr_in.
    if (inet_pton(AF_INET,
                  IP.data(),
                  &addr_server.sin_addr) != 1)
    {
        spdlog::error("Invalid server IP address");
        close(socket_fd_server);
        return 1;
    }

    // Assign our IP address and port to the socket.
    if (bind(socket_fd_server,
             (sockaddr*)&addr_server,
             sizeof(addr_server)) < 0)
    {
        spdlog::error("Failed to bind server socket");
        close(socket_fd_server);
        return 1;
    }

    // Turn the socket into a listening socket.
    // The second argument controls the connection backlog.
    if (listen(socket_fd_server, 1) < 0)
    {
        spdlog::error("Failed to listen on server socket");
        close(socket_fd_server);
        return 1;
    }

    spdlog::info("Waiting for client...");

    sockaddr_in addr_client{};
    socklen_t client_len = sizeof(addr_client);

    // Wait until a client connects.
    // accept() returns a NEW socket used to communicate with that client.
    // socket_fd_server remains the listening socket.
    int socket_fd_client = accept(socket_fd_server,
                                  (sockaddr*)&addr_client,
                                  &client_len);

    if (socket_fd_client < 0)
    {
        spdlog::error("Failed to accept client connection");
        close(socket_fd_server);
        return 1;
    }

    // Convert the client's binary IP address into a readable string.
    char client_ip[INET_ADDRSTRLEN]{};

    if (inet_ntop(AF_INET,
                  &addr_client.sin_addr,
                  client_ip,
                  sizeof(client_ip)) == nullptr)
    {
        spdlog::error("Failed to convert client IP address");
        close(socket_fd_client);
        close(socket_fd_server);
        return 1;
    }

    // Ports in sockaddr_in are stored in network byte order.
    // Convert the client's port to the host's byte order.
    int client_port = ntohs(addr_client.sin_port);

    uint16_t message_size = 0;

    try 
    {
        message_size = read_message_size(socket_fd_client);
        spdlog::info("Message size: {}", message_size);
    }
    catch (const std::exception& e)
    {
        spdlog::error("{}", e.what());

        close(socket_fd_client);
        close(socket_fd_server);
        return 1;
    }
    catch (...)
    {
        spdlog::error("Undefined error");

        close(socket_fd_client);
        close(socket_fd_server);
        return 1;
    }


    // Create exactly enough space for the incoming payload.
    std::string message_received;
    message_received.resize(message_size);

    size_t total_received = 0;

    // TCP is a byte stream. One recv() is NOT guaranteed to return
    // the entire message, so keep receiving until we have all bytes.
    while (total_received < message_received.size())
    {
        ssize_t received = recv(socket_fd_client,
                                // Continue writing after the bytes we already received.
                                message_received.data() + total_received,
                                // Ask only for the number of bytes still missing.
                                message_received.size() - total_received,
                                0);

        if (received < 0 && errno == EINTR)
            continue;

        if (received <= 0)
        {
            spdlog::error("Connection closed while receiving message");

            close(socket_fd_client);
            close(socket_fd_server);

            return 1;
        }

        // Remember how many bytes of the message we have so far.
        total_received += static_cast<size_t>(received);
    }

    spdlog::info("Received '{}' from {}:{}",
                 message_received,
                 client_ip,
                 client_port);

    // Close the connected client socket first,
    // then the server's listening socket.
    close(socket_fd_client);
    close(socket_fd_server);

    return 0;
}