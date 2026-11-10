#pragma once


#include "common/tcp_connection.hpp"

#include <string>


class ClientHandler
{
public:
    ClientHandler(Socket socket,
                  std::string client_ip,
                  int client_port);

    // Process messages until the client disconnects.
    void run();

private:
    int m_client_port;
    std::string m_client_ip;

    TcpConnection m_connection;
};