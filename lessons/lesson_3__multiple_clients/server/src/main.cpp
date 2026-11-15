#include "server/chat_room.hpp"
#include "server/chat_server.hpp"

#include "common/protocol.hpp"

#include "asio.hpp"
#include "spdlog/spdlog.h"

#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <string>


int main(int argc, char* argv[])
{
    try
    {
        const int port = argc > 1 ? std::stoi(argv[1]) : common::default_port;

        if (argc > 2 || port < 1 || port > 65535)
            throw std::invalid_argument("Usage: ChatServer [port: 1-65535]");

        asio::io_context io_context;

        server::ChatRoom room;

        server::ChatServer chat_server(io_context,
                                       static_cast<unsigned short>(port),
                                       room);

        spdlog::info("Chat server listening on port {}", port);

        io_context.run();
    }
    catch (const std::exception& error)
    {
        spdlog::error("Server error: {}", error.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}