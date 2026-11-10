#include "client/tcp_client.hpp"

#include "common/socket.hpp"

#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <sys/socket.h>


TcpClient::TcpClient(std::string server_ip, uint16_t server_port)
    : m_server_ip(std::move(server_ip)),
      m_server_port(server_port)
{
}


void TcpClient::connect()
{
    if (m_connection)
        throw std::runtime_error("Client is already connected");

    // Create an IPv4 TCP socket.
    int fd = socket(AF_INET,
                    SOCK_STREAM,
                    0);

    if (fd < 0)
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to create client socket");

    // Automatically close the socket if anything fails.
    Socket socket(fd);

    // Configure the server address.
    sockaddr_in addr_server{};

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(m_server_port);

    if (inet_pton(AF_INET,
                  m_server_ip.c_str(),
                  &addr_server.sin_addr) != 1)
    {
        throw std::runtime_error("Invalid server IP address: " + m_server_ip);
    }

    // We intentionally do not call bind().
    //
    // The operating system automatically selects
    // an available local IP address and port.
    //
    // This allows multiple client processes
    // to connect to the same server.
    if (::connect(socket.fd(),
                  reinterpret_cast<sockaddr*>(&addr_server),
                  sizeof(addr_server)) < 0)
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to connect to server");

    // Transfer socket ownership to TcpConnection.
    m_connection = std::make_unique<TcpConnection>(std::move(socket));

    spdlog::info("Connected to server {}:{}",
                 m_server_ip,
                 m_server_port);
}


void TcpClient::send_message(std::string_view message)
{
    if (!m_connection)
        throw std::runtime_error("Client is not connected");

    m_connection->send_message(message);

    spdlog::info("Sent '{}' ({} payload bytes)",
                 message,
                 message.size());
}


void TcpClient::disconnect() noexcept
{
    if (m_connection)
    {
        // Destroying TcpConnection also destroys Socket,
        // which automatically closes the file descriptor.
        m_connection.reset();

        spdlog::info("Disconnected from server");
    }
}


bool TcpClient::is_connected() const noexcept
{
    return m_connection != nullptr;
}