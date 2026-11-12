#pragma once

#include "server/i_chat_participant.hpp"

#include "boost/asio.hpp"

#include <deque>
#include <memory>
#include <string>


namespace server
{

    class IChatRoom;

    // A Session owns a single socket and its asynchronous I/O state.
    // All Session and room operations run on the server's io_context thread.
    class Session final : public IChatParticipant,
                          public std::enable_shared_from_this<Session>
    {
    public:
        Session(boost::asio::ip::tcp::socket socket,
                IChatRoom& room);

        void start();
        const std::string& name() const override;
        void deliver(const std::string& text) override;

    private:
        void read_line();
        void write_next();
        void close();

        boost::asio::ip::tcp::socket m_socket;
        IChatRoom& m_room;
        std::string m_name;
        std::string m_input;
        std::deque<std::string> m_outbox;
        bool m_joined = false;
        bool m_closed = false;
        bool m_closing_after_write = false;
    };

}
