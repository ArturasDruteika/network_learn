#include "client/tcp_client.hpp"

#include "spdlog/spdlog.h"

#include <exception>
#include <iostream>
#include <string>


constexpr int PORT_SERVER = 8000;
constexpr const char* IP_SERVER = "127.0.0.1";


int main(int argc, char* argv[])
{
    try
    {
        TcpClient client(
            IP_SERVER,
            PORT_SERVER
        );

        client.connect();

        // -----------------------------------------
        // Command-line mode
        // -----------------------------------------
        //
        // Example:
        //
        // ./client "Hello" "Second message"
        //
        if (argc > 1)
        {
            for (int i = 1; i < argc; ++i)
            {
                client.send_message(argv[i]);
            }
        }
        else
        {
            // -----------------------------------------
            // Interactive mode
            // -----------------------------------------
            //
            // Keep reading messages from standard input
            // and sending them to the server.
            //
            // Type /quit to disconnect.
            //
            spdlog::info(
                "Enter messages to send. Type /quit to exit."
            );

            std::string message;

            while (std::getline(std::cin, message))
            {
                if (message == "/quit")
                    break;

                client.send_message(message);
            }
        }

        client.disconnect();
    }
    catch (const std::exception& e)
    {
        spdlog::error(
            "Client error: {}",
            e.what()
        );

        return 1;
    }

    return 0;
}