#pragma once


#include "server/i_chat_participant.hpp"
#include "server/i_chat_room.hpp"

#include "boost/asio.hpp"

#include <deque>
#include <memory>
#include <string>


namespace server
{
    /// @brief Manages an individual client's TCP connection to the chat server.
    ///
    /// Represents a single connected client and handles asynchronous
    /// communication, including receiving messages, sending responses,
    /// and managing the connection lifecycle.
    ///
    /// Implements the IChatParticipant interface to allow interaction
    /// with the chatroom through a common abstraction.
    ///
    /// Each session owns its TCP socket and asynchronous I/O state.
    /// All session and chatroom operations execute on the server's
    /// I/O context thread.
    class Session final : public IChatParticipant,
                          public std::enable_shared_from_this<Session>
    {
    public:
        /// @brief Constructs a session for a connected TCP client.
        ///
        /// Takes ownership of the provided TCP socket and associates
        /// the session with the specified chatroom.
        ///
        /// @param socket Connected TCP socket transferred to the session.
        /// @param room Reference to the chatroom managing this participant.
        Session(boost::asio::ip::tcp::socket socket,
                IChatRoom& room);

        /// @brief Starts processing the client's connection.
        ///
        /// Initiates asynchronous reading of incoming data and begins
        /// handling the client's communication with the chat server.
        void start();

        /// @brief Retrieves the participant's nickname.
        ///
        /// @return Constant reference to the nickname associated with this session.
        const std::string& name() const override;

        /// @brief Queues a message for delivery to the connected client.
        ///
        /// Adds the provided message to the outgoing message queue
        /// and schedules asynchronous transmission when necessary.
        ///
        /// @param text Message to deliver to the connected client.
        void deliver(const std::string& text) override;

    private:
        /// @brief Asynchronously reads the next line from the client.
        ///
        /// Processes incoming protocol records, including the initial
        /// nickname registration and subsequent chat messages.
        ///
        /// Continues receiving data while the connection remains active.
        void read_line();

        /// @brief Asynchronously sends the next queued message.
        ///
        /// Retrieves the next message from the outgoing queue and
        /// initiates its transmission through the TCP socket.
        ///
        /// Ensures that outgoing messages are transmitted sequentially
        /// without overlapping asynchronous write operations.
        void write_next();

        /// @brief Closes the client's TCP connection.
        ///
        /// Terminates further communication, releases the underlying
        /// socket resources, and removes the participant from the
        /// chatroom if they were previously registered.
        void close();

    private:
        /// @brief Indicates whether the session has joined the chatroom.
        ///
        /// Tracks whether the client's nickname has been accepted
        /// and the session has been registered as an active participant.
        bool m_joined = false;

        /// @brief Indicates whether the session has been closed.
        ///
        /// Prevents repeated cleanup operations and further processing
        /// after the connection has been terminated.
        bool m_closed = false;

        /// @brief Indicates whether the connection should close after writing.
        ///
        /// Allows pending outgoing messages to be transmitted before
        /// the TCP connection is closed.
        bool m_closing_after_write = false;

        /// @brief Stores the nickname associated with the connected client.
        ///
        /// Identifies the participant within the chatroom after
        /// successful nickname registration.
        std::string m_name;

        /// @brief Stores incoming data received from the client.
        ///
        /// Acts as a buffer for assembling newline-delimited protocol
        /// records received through the TCP socket.
        std::string m_input;

        /// @brief Stores outgoing messages waiting to be transmitted.
        ///
        /// Maintains the order of messages scheduled for delivery
        /// and ensures they are sent sequentially to the client.
        std::deque<std::string> m_outbox;

        /// @brief TCP socket used for communication with the client.
        ///
        /// Owns the underlying TCP connection and handles asynchronous
        /// read and write operations throughout the session's lifetime.
        boost::asio::ip::tcp::socket m_socket;

        /// @brief Reference to the chatroom managing this session.
        ///
        /// Provides access to participant registration, removal,
        /// nickname validation, and message broadcasting through
        /// the IChatRoom interface.
        ///
        /// The chatroom is managed externally and must remain valid
        /// throughout the lifetime of the session.
        IChatRoom& m_room;
    };
}