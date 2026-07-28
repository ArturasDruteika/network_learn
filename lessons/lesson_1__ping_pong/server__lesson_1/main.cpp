#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <unistd.h>


constexpr int PORT = 8080;
constexpr std::string_view HOST = "127.0.0.1";


int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Listen only on localhost
    inet_pton(AF_INET, HOST.data(), &server_addr.sin_addr);

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 1);

    spdlog::info("Waiting for client...");

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

    char buffer[1024]{};

    recv(client_socket, buffer, sizeof(buffer), 0);

    spdlog::info("Received: {}", buffer);

    const char* reply = "Pong";

    send(client_socket, reply, strlen(reply) + 1, 0);

    close(client_socket);
    close(server_socket);

    return 0;
}