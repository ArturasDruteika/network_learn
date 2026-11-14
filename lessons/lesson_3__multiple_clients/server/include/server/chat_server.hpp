#pragma once


#include "i_chat_room.hpp"
#include "boost/asio.hpp"


namespace server
{
    /// @brief Manages incoming TCP connections to the chat server.
    ///
    /// Listens for client connections on the specified port and creates
    /// a dedicated session for each connected client.
    ///
    /// The server delegates chatroom operations to an injected IChatRoom
    /// instance, separating connection management from chatroom behavior.
    class ChatServer final
    {
    public:
        /// @brief Constructs a chat server using the provided I/O context.
        ///
        /// Initializes the TCP acceptor to listen for incoming connections
        /// on the specified port and associates the server with a chatroom.
        ///
        /// @param io_context Boost.Asio I/O context used for asynchronous operations.
        /// @param port TCP port on which the server listens for connections.
        /// @param room Reference to the chatroom responsible for managing participants.
        ChatServer(boost::asio::io_context& io_context,
                   unsigned short port,
                   IChatRoom& room);

    private:
        /// @brief Asynchronously accepts the next incoming client connection.
        ///
        /// Waits for a new TCP connection and creates a session for the
        /// connected client.
        ///
        /// Continues accepting additional connections while the TCP
        /// acceptor remains open.
        void accept_next();

    private:
        /// @brief TCP acceptor responsible for listening for client connections.
        ///
        /// Manages the server's listening socket and asynchronously
        /// accepts incoming TCP connections.
        boost::asio::ip::tcp::acceptor m_acceptor;

        /// @brief Reference to the chatroom used by connected client sessions.
        ///
        /// Provides access to participant management and message broadcasting
        /// through the IChatRoom interface.
        ///
        /// The chatroom is provided externally and must remain valid
        /// throughout the lifetime of the server.
        IChatRoom& m_room;
    };
}