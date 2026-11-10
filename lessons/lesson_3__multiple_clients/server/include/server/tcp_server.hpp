#pragma once


#include "common/socket.hpp"

#include <cstdint>
#include <string>


class TcpServer
{
public:
    TcpServer(std::string ip, uint16_t port);

    // Create, bind, and configure the listening socket.
    void start();

    // Accept and handle incoming clients.
    //
    // This function runs indefinitely.
    void run();

private:
    uint16_t m_port;
    std::string m_ip;

    Socket m_listening_socket;
};