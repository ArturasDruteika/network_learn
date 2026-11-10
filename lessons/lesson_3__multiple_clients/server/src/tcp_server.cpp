#include "server/tcp_server.hpp"
#include "server/client_handler.hpp"

#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cerrno>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>
#include <sys/socket.h>


TcpServer::TcpServer(std::string ip, uint16_t port)
    : m_ip(std::move(ip)),
      m_port(port)
{
}


void TcpServer::start()
{
    // Create an IPv4 TCP socket.
    int fd = ::socket(AF_INET,
                      SOCK_STREAM,
                      0);

    if (fd < 0)
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to create server socket");

    // Transfer ownership to our RAII socket.
    m_listening_socket = Socket(fd);

    // Allow the address to be reused after restarting.
    int reuse_address = 1;

    if (::setsockopt(m_listening_socket.fd(),
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &reuse_address,
                     sizeof(reuse_address)) < 0)
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to set SO_REUSEADDR");

    // Configure the server address.
    sockaddr_in addr_server{};

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(m_port);

    if (::inet_pton(AF_INET,
                    m_ip.c_str(),
                    &addr_server.sin_addr) != 1)
        throw std::runtime_error("Invalid server IP address: " + m_ip);

    // Assign the IP address and port to the socket.
    if (::bind(m_listening_socket.fd(),
               reinterpret_cast<sockaddr*>(&addr_server),
               sizeof(addr_server)) < 0)
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to bind server socket");

    // Start listening for incoming connections.
    if (::listen(m_listening_socket.fd(),
                 SOMAXCONN) < 0)
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to listen on server socket");

    spdlog::info("Server listening on {}:{}",
                 m_ip,
                 m_port);
}


void TcpServer::run()
{
    if (!m_listening_socket.valid())
        throw std::runtime_error("Server has not been started");

    // Keep client threads alive while the server runs.
    std::vector<std::jthread> client_threads;

    // Continuously accept new clients.
    while (true)
    {
        sockaddr_in addr_client{};
        socklen_t client_len = sizeof(addr_client);

        int client_fd = ::accept(m_listening_socket.fd(),
                                 reinterpret_cast<sockaddr*>(&addr_client),
                                 &client_len);

        if (client_fd < 0)
        {
            if (errno == EINTR)
                continue;

            spdlog::error("Failed to accept client: {}",
                          std::generic_category().message(errno));

            continue;
        }

        // The accepted socket is now owned by this object.
        Socket client_socket(client_fd);

        // Convert the client's IP address to readable text.
        char client_ip[INET_ADDRSTRLEN]{};

        if (::inet_ntop(AF_INET,
                        &addr_client.sin_addr,
                        client_ip,
                        sizeof(client_ip)) == nullptr)
        {
            spdlog::error("Failed to convert client IP address");

            continue;
        }

        // Convert the client's port to host byte order.
        int client_port = ntohs(addr_client.sin_port);

        try
        {
            // Create a separate jthread for this client.
            //
            // Ownership of the connected socket is
            // transferred into the thread.
            client_threads.emplace_back(
                [socket = std::move(client_socket),
                 ip = std::string(client_ip),
                 client_port]() mutable
                {
                    try
                    {
                        ClientHandler handler(std::move(socket),
                                              std::move(ip),
                                              client_port);

                        handler.run();
                    }
                    catch (const std::exception& e)
                    {
                        spdlog::error("Client handler failed: {}",
                                      e.what());
                    }
                    catch (...)
                    {
                        spdlog::error("Client handler failed: unknown error");
                    }
                }
            );
        }
        catch (const std::exception& e)
        {
            spdlog::error("Failed to start client thread: {}",
                          e.what());
        }
    }
}