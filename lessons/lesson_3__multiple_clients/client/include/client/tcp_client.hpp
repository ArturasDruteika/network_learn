#pragma once

#include "common/tcp_connection.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>


class TcpClient
{
public:
    TcpClient(std::string server_ip, uint16_t server_port);

    // Establish a TCP connection to the server.
    void connect();

    // Send one message over the existing connection.
    void send_message(std::string_view message);

    // Close the connection.
    void disconnect() noexcept;

    // Check whether the client owns a connection.
    bool is_connected() const noexcept;

private:
    std::string m_server_ip;
    uint16_t m_server_port;

    std::unique_ptr<TcpConnection> m_connection;
};