#include "server/session.hpp"

#include "common/protocol.hpp"
#include "server/chat_room.hpp"

#include <utility>


namespace server
{

    // namespace boost::asio = boost::boost::asio;
    // using boost::asio::ip::tcp = boost::asio::ip::boost::asio::ip::tcp;

    Session::Session(boost::asio::ip::tcp::socket socket, IChatRoom &room)
        : m_socket(std::move(socket))
        , m_room(room)
    {
    }

    void Session::start()
    {
        read_line();
    }

    const std::string &Session::name() const
    {
        return m_name;
    }

    void Session::deliver(const std::string &text)
    {
        if (m_closed)
            return;

        const bool busy = !m_outbox.empty();
        m_outbox.push_back(text + "\n");

        if (!busy)
            write_next();
    }

    void Session::read_line()
    {
        if (m_closed || m_closing_after_write)
            return;

        auto self = shared_from_this();

        boost::asio::async_read_until(m_socket,
                                      boost::asio::dynamic_buffer(m_input, common::max_line_buffer),
                                      '\n',
                                      [self](boost::system::error_code error, std::size_t bytes)
                                      {
                                          if (error)
                                          {
                                              self->close();
                                              return;
                                          }

                                          auto line = common::strip_cr(self->m_input.substr(0, bytes - 1));
                                          self->m_input.erase(0, bytes);

                                          if (!self->m_joined)
                                          {
                                              if (!common::valid_name(line) || self->m_room.name_taken(line))
                                              {
                                                  self->deliver("[server] Invalid or already-used nickname; disconnecting.");
                                                  self->m_closing_after_write = true;
                                                  return;
                                              }

                                              self->m_name = std::move(line);
                                              self->m_joined = true;
                                              self->m_room.join(self);
                                              self->deliver("[server] Welcome, " + self->m_name + "!");
                                              self->m_room.broadcast("[server] " + self->m_name + " joined the room.");
                                          }
                                          else if (line.size() > common::max_message_length)
                                          {
                                              self->deliver("[server] Message too long (maximum 2048 bytes).");
                                          }
                                          else if (!line.empty())
                                          {
                                              self->m_room.broadcast("[" + self->m_name + "] " + line);
                                          }

                                          self->read_line();
                                      });
    }

    void Session::write_next()
    {
        auto self = shared_from_this();

        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_outbox.front()),
                                 [self](boost::system::error_code error, std::size_t)
                                 {
                                     if (error)
                                     {
                                         self->close();
                                         return;
                                     }

                                     self->m_outbox.pop_front();

                                     if (!self->m_outbox.empty())
                                         self->write_next();
                                     else if (self->m_closing_after_write)
                                         self->close();
                                 });
    }

    void Session::close()
    {
        if (m_closed)
            return;

        m_closed = true;
        boost::system::error_code ignored;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
        m_socket.close(ignored);

        if (m_joined)
        {
            m_joined = false;
            m_room.leave(shared_from_this());
            m_room.broadcast("[server] " + m_name + " left the room.");
        }
    }

}
