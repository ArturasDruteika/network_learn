#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

const int PORT = 8080;

void receiveMessages(int socket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(socket, buffer, sizeof(buffer), 0);

        if (bytes <= 0) {
            std::cout << "\nServer disconnected.\n";
            break;
        }

        std::cout << "\nServer: " << buffer << std::endl;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Connection failed.\n";
        return 1;
    }

    std::cout << "Connected to server!\n";

    std::thread receiver(receiveMessages, sock);

    std::string message;
    while (true) {
        std::getline(std::cin, message);

        if (message == "exit")
            break;

        send(sock, message.c_str(), message.size() + 1, 0);
    }

    close(sock);

    receiver.join();
}