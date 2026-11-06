#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <limits>
#include <unistd.h>
#include <vector>


// Server address we want to connect to.
constexpr int PORT_SERVER = 8000;
constexpr std::string_view IP_SERVER = "127.0.0.1";

// Address that we explicitly assign to our client socket.
constexpr int PORT_CLIENT = 9000;
constexpr std::string_view IP_CLIENT = "127.0.0.2";

// Number of bytes used to represent the message length.
constexpr uint16_t BYTES_MESSAGE_LENGTH = 2;

static_assert(BYTES_MESSAGE_LENGTH == sizeof(uint16_t));


// Encode a message using our simple protocol:
//
// [ 2 bytes: message size ][ message bytes... ]
//
// Example for "Hello":
//
// [0x00][0x05][H][e][l][l][o]
//
// Because the message size is stored in two bytes (uint16_t),
// our protocol can represent message sizes from 0 to 65535.
//
// The message size is encoded in network byte order (big-endian).
std::vector<uint8_t> encode_message(const std::string& message)
{
    size_t message_length = message.size();

    // Make sure the message length fits into two bytes.
    if (message_length > std::numeric_limits<uint16_t>::max())
    {
        throw std::runtime_error("Message size over the limit. Maximum possible message size: " +
                                 std::to_string(UINT16_MAX) +
                                 ", current message size: " +
                                 std::to_string(message_length));
    }

    std::vector<uint8_t> message_bytes;

    // Reserve memory for:
    //
    //   2 bytes for the message size
    //   +
    //   N bytes for the actual message
    //
    // reserve() allocates capacity but does not add elements.
    message_bytes.reserve(BYTES_MESSAGE_LENGTH + message_length);

    // Convert the message size to a 16-bit unsigned integer.
    uint16_t message_size = static_cast<uint16_t>(message_length);

    // Store the message size in network byte order (big-endian).
    //
    // First byte:  most significant 8 bits.
    // Second byte: least significant 8 bits.
    //
    // Example:
    //
    // Message size = 300 (0x012C)
    //
    // First byte  = 0x01
    // Second byte = 0x2C
    message_bytes.push_back(static_cast<uint8_t>((message_size >> 8) & 0xFF));
    message_bytes.push_back(static_cast<uint8_t>(message_size & 0xFF));

    // Append the actual message bytes after the size bytes.
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

    if (socket_fd_client < 0)
    {
        spdlog::error("Failed to create socket");
        return 1;
    }


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
    if (inet_pton(AF_INET,
                  IP_CLIENT.data(),
                  &addr_client.sin_addr) != 1)
    {
        spdlog::error("Invalid client IP address");
        close(socket_fd_client);
        return 1;
    }

    // Explicitly assign our client socket an IP address and port.
    //
    // This is optional for a normal TCP client. If bind() is omitted,
    // the operating system chooses the local IP and an available port.
    if (bind(socket_fd_client,
             (sockaddr*)&addr_client,
             sizeof(addr_client)) < 0)
    {
        spdlog::error("Failed to bind client socket");
        close(socket_fd_client);
        return 1;
    }


    // --------------------
    // Server setup
    // --------------------

    sockaddr_in addr_server{};

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(PORT_SERVER);

    // Convert the server IP address into binary IPv4 format.
    if (inet_pton(AF_INET,
                  IP_SERVER.data(),
                  &addr_server.sin_addr) != 1)
    {
        spdlog::error("Invalid server IP address");
        close(socket_fd_client);
        return 1;
    }


    // Establish a TCP connection to the server.
    //
    // After connect() succeeds, socket_fd_client represents
    // the TCP connection between this client and the server.
    if (connect(socket_fd_client,
                (sockaddr*)&addr_server,
                sizeof(addr_server)) < 0)
    {
        spdlog::error("Failed to connect to server");
        close(socket_fd_client);
        return 1;
    }


    // Messages we can send to the server.
    const std::string message_1 = "I contain - 20 bytes";

    // Convert our message into:
    //
    // [ 2 bytes: message size ][ message data ]
    //
    // For example:
    //
    // "Hello"
    //
    // becomes:
    //
    // [0x00][0x05][H][e][l][l][o]
    std::vector<uint8_t> message_bytes = encode_message(message_1);


    // Send the encoded bytes through the TCP connection.
    //
    // TCP may send fewer bytes than requested in a single send().
    // Keep sending until the entire encoded message has been sent.
    size_t total_sent = 0;

    while (total_sent < message_bytes.size())
    {
        ssize_t sent = send(socket_fd_client,
                            message_bytes.data() + total_sent,
                            message_bytes.size() - total_sent,
                            0);

        if (sent < 0 && errno == EINTR)
            continue;

        if (sent <= 0)
        {
            spdlog::error("Failed to send message");
            close(socket_fd_client);
            return 1;
        }

        // Remember how many bytes we have sent so far.
        total_sent += static_cast<size_t>(sent);
    }

    spdlog::info("Sent '{}' ({} payload bytes)",
                 message_1,
                 message_1.size());

    // Close our side of the TCP connection.
    close(socket_fd_client);

    return 0;
}