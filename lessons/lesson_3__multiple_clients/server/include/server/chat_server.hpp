#pragma once

#include "boost/asio.hpp"


namespace server
{

    class IChatRoom;

    // Accepts connections and constructs sessions; room behavior is injected.
    class ChatServer final
    {
    public:
        ChatServer(boost::asio::io_context& io_context,
                   unsigned short port,
                   IChatRoom& room);

    private:
        void accept_next();

        boost::asio::ip::tcp::acceptor m_acceptor;
        IChatRoom& m_room;
    };

}
