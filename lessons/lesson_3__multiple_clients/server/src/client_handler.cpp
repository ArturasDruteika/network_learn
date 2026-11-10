#include "server/client_handler.hpp"

#include "spdlog/spdlog.h"

#include <exception>
#include <utility>


ClientHandler::ClientHandler(Socket socket,
                             std::string client_ip,
                             int client_port)
    : m_connection(std::move(socket))
    , m_client_ip(std::move(client_ip))
    , m_client_port(client_port)
{
}


void ClientHandler::run()
{
    spdlog::info("Client connected: {}:{}",
                 m_client_ip,
                 m_client_port);

    try
    {
        // Keep receiving messages from this client.
        //
        // Each receive_message() call reads exactly one
        // length-prefixed message.
        //
        // The loop ends when the client disconnects.
        while (true)
        {
            std::string message;

            if (!m_connection.receive_message(message))
                // The client closed its connection cleanly.
                break;

            spdlog::info("Received '{}' from {}:{} ({} bytes)",
                         message,
                         m_client_ip,
                         m_client_port,
                         message.size());

            // Application-specific message processing
            // can be added here.
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Client {}:{} - Error: {}",
                      m_client_ip,
                      m_client_port,
                      e.what());
    }
    catch (...)
    {
        spdlog::error("Client {}:{} - Unknown error",
                      m_client_ip,
                      m_client_port);
    }

    spdlog::info("Client disconnected: {}:{}",
                 m_client_ip,
                 m_client_port);

    // The TcpConnection destructor automatically
    // closes the connected socket.
}