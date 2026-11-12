#include "server/chat_room.hpp"
#include "server/chat_server.hpp"

#include "common/protocol.hpp"

#include "boost/asio.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>


int main(int argc, char* argv[])
{
    try
    {
        const int port = argc > 1 ? std::stoi(argv[1]) : common::default_port;

        if (argc > 2 || port < 1 || port > 65535)
            throw std::invalid_argument("Usage: ChatServer [port: 1-65535]");

        boost::asio::io_context io_context;
        server::ChatRoom room;
        server::ChatServer ChatServer(io_context,
                                        static_cast<unsigned short>(port),
                                        room);

        std::cout << "Chat server listening on port " << port << '\n';
        io_context.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Server error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
