#include "server/chat_server.hpp"

#include "server/i_chat_room.hpp"
#include "server/session.hpp"

#include <memory>
#include <utility>


namespace server
{
    ChatServer::ChatServer(boost::asio::io_context &io_context,
                           unsigned short port,
                           IChatRoom &room)
        : m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          m_room(room)
    {
        accept_next();
    }

    void ChatServer::accept_next()
    {
        m_acceptor.async_accept(
            [this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
            {
                if (!error)
                    std::make_shared<Session>(std::move(socket), m_room)->start();

                if (m_acceptor.is_open())
                    accept_next();
            });
    }

}
