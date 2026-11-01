#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>


// Server address we want to connect to.
constexpr int PORT_SERVER = 8000;
constexpr std::string_view IP_SERVER = "127.0.0.1";

// Address that we explicitly assign to our client socket.
constexpr int PORT_CLIENT = 9000;
constexpr std::string_view IP_CLIENT = "127.0.0.2";


// Encode a message using our simple protocol:
//
// [ 1 byte: message size ][ message bytes... ]
//
// Example for "Hello":
//
// [5][H][e][l][l][o]
//
// Because the message size is stored in one byte (uint8_t),
// our protocol can represent message sizes only from 0 to 255.
std::vector<uint8_t> encode_message(const std::string& message)
{
    size_t message_length = message.size();

    // Make sure the message length fits into one byte.
    if (message_length > UINT8_MAX - 1)
    {
        throw std::runtime_error(
            "Message size over the limit. Maximum possible message size: " +
            std::to_string(UINT8_MAX) +
            ", current message size: " + std::to_string(message_length));
    }

    std::vector<uint8_t> message_bytes;

    // Reserve memory for:
    //
    //   1 byte for the message size
    //   +
    //   N bytes for the actual message
    //
    // reserve() allocates capacity but does not add elements.
    message_bytes.reserve(1 + message_length);

    // Store the message size as the first byte.
    message_bytes.push_back(static_cast<uint8_t>(message_length));

    // Append the actual message bytes after the size byte.
    message_bytes.insert(message_bytes.end(),
                         message.begin(),
                         message.end());

    return message_bytes;
}


int main()
{
    // Create an IPv4 TCP socket.
    int socket_fd_client = socket(AF_INET,
                                  SOCK_STREAM,
                                  0);


    // --------------------
    // Client setup
    // --------------------

    sockaddr_in addr_client{};

    // Use IPv4.
    addr_client.sin_family = AF_INET;

    // sockaddr_in expects the port in network byte order.
    addr_client.sin_port = htons(PORT_CLIENT);

    // Convert "127.0.0.2" from text into the binary IPv4 format
    // required by sockaddr_in.
    inet_pton(AF_INET,
              IP_CLIENT.data(),
              &addr_client.sin_addr);

    // Explicitly assign our client socket an IP address and port.
    //
    // This is optional for a normal TCP client. If bind() is omitted,
    // the operating system chooses the local IP and an available port.
    bind(socket_fd_client,
         (sockaddr*)&addr_client,
         sizeof(addr_client));


    // --------------------
    // Server setup
    // --------------------

    sockaddr_in addr_server{};

    addr_server.sin_family = AF_INET;

    // Convert the server port to network byte order.
    addr_server.sin_port = htons(PORT_SERVER);

    // Convert the server IP address into binary IPv4 format.
    inet_pton(AF_INET,
              IP_SERVER.data(),
              &addr_server.sin_addr);


    // Establish a TCP connection to the server.
    //
    // After connect() succeeds, socket_fd_client represents
    // the TCP connection between this client and the server.
    connect(socket_fd_client,
            (sockaddr*)&addr_server,
            sizeof(addr_server));


    // Messages we can send to the server.
    const std::string message_1 = "I contain - 20 bytes";

    // Convert our message into:
    //
    // [ message size ][ message data ]
    //
    // For example:
    //
    // "Hello"
    //
    // becomes:
    //
    // [5][H][e][l][l][o]
    std::vector<uint8_t> message_bytes = encode_message(message_1);

    // Send the encoded bytes through the TCP connection.
    //
    // data() -> pointer to the first byte
    // size() -> number of bytes we want to send
    send(socket_fd_client,
         message_bytes.data(),
         message_bytes.size(),
         0);

    // Close our side of the TCP connection.
    close(socket_fd_client);

    return 0;
}
