#include "common/tcp_connection.hpp"
#include "common/message_protocol.hpp"

#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <system_error>
#include <vector>
#include <sys/socket.h>


TcpConnection::TcpConnection(Socket socket) noexcept
    : m_socket(std::move(socket))
{
}


void TcpConnection::send_message(std::string_view message)
{
    std::vector<uint8_t> bytes = MessageProtocol::encode(message);

    std::size_t total_sent = 0;

    while (total_sent < bytes.size())
    {
        ssize_t sent = send(m_socket.fd(),
                            bytes.data() + total_sent,
                            bytes.size() - total_sent,
                            MSG_NOSIGNAL);

        if (sent < 0 && errno == EINTR)
            continue;

        if (sent < 0)
        {
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to send message");
        }

        if (sent == 0)
            throw std::runtime_error("Socket send returned zero bytes");

        total_sent += static_cast<std::size_t>(sent);
    }
}


bool TcpConnection::receive_exact(void* buffer, std::size_t size)
{
    auto* bytes = static_cast<uint8_t*>(buffer);

    std::size_t total_received = 0;

    while (total_received < size)
    {
        ssize_t received = recv(m_socket.fd(),
                                bytes + total_received,
                                size - total_received,
                                0);

        if (received < 0 && errno == EINTR)
            continue;

        if (received < 0)
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to receive data");

        if (received == 0)
        {
            if (total_received == 0)
                return false;

            throw std::runtime_error("Connection closed during message reception");
        }

        total_received += static_cast<std::size_t>(received);
    }

    return true;
}


bool TcpConnection::receive_message(std::string& message)
{
    uint8_t length_bytes[MessageProtocol::LENGTH_BYTES]{};

    // First read the two-byte length prefix.
    if (!receive_exact(length_bytes, sizeof(length_bytes)))
        return false;

    const uint16_t message_size = MessageProtocol::decode_length(length_bytes);

    // Allocate exactly enough space for the payload.
    std::string received_message(message_size, '\0');

    // An empty message is valid.
    if (message_size > 0)
    {
        if (!receive_exact(received_message.data(), message_size))
            throw std::runtime_error("Connection closed before message payload was received");
    }

    message = std::move(received_message);

    return true;
}