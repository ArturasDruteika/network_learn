#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

const int port = 8080;

int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr));

    const char* message = "Ping";

    send(socket_fd, message, strlen(message) + 1, 0);

    char buffer[1024]{};

    recv(socket_fd, buffer, sizeof(buffer), 0);

    spdlog::info("Server replied: {}", buffer);

    close(socket_fd);

    return 0;
}