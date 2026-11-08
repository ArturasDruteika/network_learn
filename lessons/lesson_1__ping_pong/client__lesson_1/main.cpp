#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <string_view>
#include <unistd.h>


constexpr int PORT_SERVER = 8080;
constexpr std::string_view IP_SERVER = "127.0.0.1";

constexpr int PORT_CLIENT = 9000;
constexpr std::string_view IP_CLIENT = "127.0.0.2";


int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Client's own address
    sockaddr_in client_addr{};
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT_CLIENT);

    inet_pton(AF_INET,
              IP_CLIENT.data(),
              &client_addr.sin_addr);

    bind(socket_fd,
         (sockaddr*)&client_addr,
         sizeof(client_addr));

    // Server's address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_SERVER);

    inet_pton(AF_INET,
              IP_SERVER.data(),
              &server_addr.sin_addr);

    connect(socket_fd,
            (sockaddr*)&server_addr,
            sizeof(server_addr));

    const char* message = "ping";

    send(socket_fd,
         message,
         strlen(message) + 1,
         0);

    char buffer[1024]{};

    recv(socket_fd,
         buffer,
         sizeof(buffer),
         0);

    spdlog::info("Server replied: {}", buffer);

    close(socket_fd);

    return 0;
}