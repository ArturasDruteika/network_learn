#include "client/chat_client.hpp"

#include "common/protocol.hpp"

#include "asio.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>


int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cerr << "Usage: ChatClient <host> <nickname> [port]\n";
        return EXIT_FAILURE;
    }

    const std::string nickname = argv[2];

    if (!common::valid_name(nickname))
    {
        std::cerr << "Nickname must be 1-24 letters, digits, '_' or '-'.\n";
        return EXIT_FAILURE;
    }

    try
    {
        asio::io_context io_context;
        std::shared_ptr<client::ChatClient> ChatClient = std::make_shared<client::ChatClient>(io_context);

        ChatClient->connect(argv[1],
                            argc == 4 ? argv[3] : std::to_string(common::default_port),
                            nickname);

        asio::executor_work_guard<asio::io_context::executor_type, void, void> work_guard = asio::make_work_guard(io_context);
        std::thread network_thread([&io_context]()
                                   {
                                       io_context.run();
                                   });

        std::cout << "Connected as " << nickname << ". Type messages, or /quit to exit.\n";

        std::string line;

        while (std::getline(std::cin, line))
        {
            if (line == "/quit")
                break;

            if (line.size() > common::max_message_length)
            {
                std::cerr << "Message too long (maximum 2048 bytes).\n";
                continue;
            }

            if (!line.empty())
            {
                asio::post(io_context,
                                  [ChatClient, line]()
                                  {
                                      ChatClient->send(line);
                                  });
            }
        }

        asio::post(io_context,
                          [ChatClient]()
                          {
                              ChatClient->stop();
                          });

        work_guard.reset();
        network_thread.join();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Client error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
