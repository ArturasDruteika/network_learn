#include "client/chat_client.hpp"

#include "common/protocol.hpp"

#include <iostream>


namespace client
{
    ChatClient::ChatClient(boost::asio::io_context &io_context)
        : m_socket(io_context)
    {
    }

    void ChatClient::connect(const std::string &host,
                             const std::string &port,
                             const std::string &nickname)
    {
        boost::asio::ip::tcp::resolver resolver(m_socket.get_executor());
        boost::asio::connect(m_socket,
                             resolver.resolve(host, port));
        send(nickname);
        read_line();
    }

    void ChatClient::send(const std::string &line)
    {
        if (m_stopped)
            return;

        const bool busy = !m_outbox.empty();
        m_outbox.push_back(line + "\n");

        if (!busy)
            write_next();
    }

    void ChatClient::stop()
    {
        if (m_stopped)
            return;

        m_stopped = true;
        boost::system::error_code ignored;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
        m_socket.close(ignored);
    }

    void ChatClient::read_line()
    {
        auto self = shared_from_this();

        boost::asio::async_read_until(m_socket,
                                      boost::asio::dynamic_buffer(m_input, common::max_line_buffer),
                                      '\n',
                                      [self](boost::system::error_code error, std::size_t bytes)
                                      {
                                          if (error)
                                          {
                                              if (!self->m_stopped)
                                                  std::cout << "[client] Disconnected: " << error.message() << '\n';

                                              self->stop();
                                              return;
                                          }

                                          std::cout << common::strip_cr(self->m_input.substr(0, bytes - 1)) << '\n';
                                          self->m_input.erase(0, bytes);
                                          self->read_line();
                                      });
    }

    void ChatClient::write_next()
    {
        auto self = shared_from_this();

        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_outbox.front()),
                                 [self](boost::system::error_code error, std::size_t)
                                 {
                                     if (error)
                                     {
                                         self->stop();
                                         return;
                                     }

                                     self->m_outbox.pop_front();

                                     if (!self->m_outbox.empty())
                                         self->write_next();
                                 });
    }
}
