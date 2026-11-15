#include "client/chat_client.hpp"
#include "common/protocol.hpp"

#include "spdlog/spdlog.h"


namespace client
{
    ChatClient::ChatClient(asio::io_context& io_context)
        : m_socket(io_context)
    {
    }


    void ChatClient::connect(const std::string& host,
                             const std::string& port,
                             const std::string& nickname)
    {
        asio::ip::tcp::resolver resolver(m_socket.get_executor());

        asio::connect(m_socket,
                      resolver.resolve(host, port));

        send(nickname);
        read_line();
    }


    void ChatClient::send(const std::string& line)
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
        std::error_code ignored;

        m_socket.shutdown(asio::ip::tcp::socket::shutdown_both,
                          ignored);

        m_socket.close(ignored);
    }


    void ChatClient::read_line()
    {
        std::shared_ptr<client::ChatClient> self = shared_from_this();

        asio::async_read_until(m_socket,
                               asio::dynamic_buffer(m_input, common::max_line_buffer),
                               '\n',
                               [self](std::error_code error, std::size_t bytes)
                               {
                                   if (error)
                                   {
                                       if (!self->m_stopped)
                                           spdlog::warn("[client] Disconnected: {}", error.message());

                                       self->stop();
                                       return;
                                   }

                                   spdlog::info("{}",
                                                common::strip_cr(self->m_input.substr(0, bytes - 1)));

                                   self->m_input.erase(0, bytes);
                                   self->read_line();
                               });
    }


    void ChatClient::write_next()
    {
        std::shared_ptr<client::ChatClient> self = shared_from_this();

        asio::async_write(m_socket,
                          asio::buffer(m_outbox.front()),
                          [self](std::error_code error, std::size_t)
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