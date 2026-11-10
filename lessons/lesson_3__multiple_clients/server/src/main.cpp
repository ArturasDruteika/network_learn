#include "server/tcp_server.hpp"

#include "spdlog/spdlog.h"

#include <exception>


constexpr int PORT_SERVER = 8000;
constexpr const char* IP_SERVER = "127.0.0.1";


int main()
{
    try
    {
        TcpServer server(IP_SERVER,
                         PORT_SERVER);

        server.start();

        server.run();
    }
    catch (const std::exception& e)
    {
        spdlog::error("Server error: {}",
                      e.what());

        return 1;
    }

    return 0;
}