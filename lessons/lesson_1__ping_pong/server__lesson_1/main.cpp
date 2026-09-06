#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <string_view>
#include <unistd.h>


constexpr int PORT = 8080;
constexpr std::string_view IP = "127.0.0.1";


int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Listen only on localhost
    inet_pton(AF_INET, IP.data(), &server_addr.sin_addr);

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));

    listen(server_socket, 1);

    spdlog::info("Waiting for client...");

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(
        server_socket,
        (sockaddr*)&client_addr,
        &client_len
    );


    // Get client's IP address
    char client_ip[INET_ADDRSTRLEN]{};

    inet_ntop(
        AF_INET,
        &client_addr.sin_addr,
        client_ip,
        sizeof(client_ip)
    );

    // Get client's port
    int client_port = ntohs(client_addr.sin_port);

    char buffer[1024]{};

    recv(client_socket, buffer, sizeof(buffer), 0);

    spdlog::info(
        "Received '{}' from {}:{}",
        buffer,
        client_ip,
        client_port
    );


    const char* reply = "pong";

    send(client_socket, reply, strlen(reply), 0);


    close(client_socket);
    close(server_socket);

    return 0;
}