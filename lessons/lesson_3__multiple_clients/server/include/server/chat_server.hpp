#pragma once

#include "i_chat_room.hpp"
#include "boost/asio.hpp"


namespace server
{
    // Accepts connections and constructs sessions; room behavior is injected.
    class ChatServer final
    {
    public:
        ChatServer(boost::asio::io_context& io_context,
                   unsigned short port,
                   IChatRoom& room);

    private:
        void accept_next();

    private:
        boost::asio::ip::tcp::acceptor m_acceptor;
        IChatRoom& m_room;
    };
}
