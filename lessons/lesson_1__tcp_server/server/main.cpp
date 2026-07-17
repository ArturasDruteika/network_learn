#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

const int port = 8080;

void receive_messages(int socket_fd)
{
    char buffer[1024];

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0)
        {
            std::cout << "\nClient disconnected.\n";
            break;
        }

        std::cout << "\nClient: " << buffer << std::endl;
    }
}

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 1);

    std::cout << "Waiting for client on port " << port << "...\n";

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

    std::cout << "Client connected!\n";

    std::thread receiver_thread(receive_messages, client_socket);

    std::string message;

    while (true)
    {
        std::getline(std::cin, message);

        if (message == "exit")
            break;

        send(client_socket, message.c_str(), message.size() + 1, 0);
    }

    close(client_socket);
    close(server_socket);

    receiver_thread.join();

    return 0;
}