#pragma once

#include "boost/asio.hpp"

#include <deque>
#include <memory>
#include <string>


namespace client
{
    // Networking only; main.cpp handles terminal input and display policy.
    class ChatClient final 
        : public std::enable_shared_from_this<ChatClient>
    {
    public:
        explicit ChatClient(boost::asio::io_context& io_context);

        void connect(const std::string& host,
                     const std::string& port,
                     const std::string& nickname);

        // Run on the io_context thread (or before io_context.run()).
        void send(const std::string& line);
        void stop();

    private:
        void read_line();
        void write_next();

    private:
        bool m_stopped = false;
        std::string m_input;
        std::deque<std::string> m_outbox;
        boost::asio::ip::tcp::socket m_socket;
    };
}
