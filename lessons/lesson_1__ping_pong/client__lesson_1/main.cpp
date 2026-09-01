#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <string_view>
#include <unistd.h>


constexpr int SERVER_PORT = 8080;
constexpr std::string_view SERVER_IP = "127.0.0.1";

constexpr int CLIENT_PORT = 9000;
constexpr std::string_view CLIENT_IP = "127.0.0.2";


int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Client's own address
    sockaddr_in client_addr{};
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);

    inet_pton(AF_INET, CLIENT_IP.data(), &client_addr.sin_addr);

    bind(socket_fd, (sockaddr*)&client_addr, sizeof(client_addr));

    // Server's address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    inet_pton(AF_INET, SERVER_IP.data(), &server_addr.sin_addr);

    connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr));

    const char* message = "ping";

    send(socket_fd, message, strlen(message) + 1, 0);

    char buffer[1024]{};

    recv(socket_fd, buffer, sizeof(buffer), 0);

    spdlog::info("Server replied: {}", buffer);

    close(socket_fd);

    return 0;
}