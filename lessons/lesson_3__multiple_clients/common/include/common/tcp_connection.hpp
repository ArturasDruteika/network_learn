#pragma once

#include "common/socket.hpp"

#include <cstddef>
#include <string>
#include <string_view>


class TcpConnection
{
public:
    explicit TcpConnection(Socket socket) noexcept;

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    TcpConnection(TcpConnection&&) noexcept = default;
    TcpConnection& operator=(TcpConnection&&) noexcept = default;

    // Send one complete length-prefixed message.
    void send_message(std::string_view message);

    // Receive one complete length-prefixed message.
    //
    // Returns false when the peer closes the connection
    // cleanly between messages.
    //
    // Throws if the connection closes in the middle
    // of a message or a socket operation fails.
    bool receive_message(std::string& message);

private:
    // Receive exactly 'size' bytes.
    //
    // Returns false only if the peer disconnects
    // before any bytes are received.
    bool receive_exact(void* buffer, std::size_t size);

    Socket m_socket;
};