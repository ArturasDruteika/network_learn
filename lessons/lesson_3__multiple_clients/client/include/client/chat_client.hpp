#pragma once

#include "boost/asio.hpp"

#include <deque>
#include <memory>
#include <string>


namespace client
{
    /// @brief Manages an asynchronous TCP connection to the chat server.
    ///
    /// Handles establishing the connection, sending messages, receiving
    /// incoming data, and closing the connection.
    ///
    /// This class is responsible only for networking. Terminal input
    /// and output policies are managed by main.cpp.
    class ChatClient final 
        : public std::enable_shared_from_this<ChatClient>
    {
    public:
        /// @brief Constructs a chat client using the provided I/O context.
        ///
        /// @param io_context Boost.Asio I/O context used for asynchronous operations.
        explicit ChatClient(boost::asio::io_context& io_context);

        /// @brief Establishes a connection to the chat server.
        ///
        /// Resolves the specified host and port, connects to the server,
        /// and registers the client using the provided nickname.
        ///
        /// @param host Hostname or IP address of the chat server.
        /// @param port Port number or service name of the chat server.
        /// @param nickname Nickname used to identify the client in the chatroom.
        void connect(const std::string& host,
                     const std::string& port,
                     const std::string& nickname);

        /// @brief Queues a message to be sent to the chat server.
        ///
        /// Schedules the message for asynchronous transmission.
        /// Must be called on the I/O context thread or before
        /// io_context.run() is started.
        ///
        /// @param line Message to send to the server.
        void send(const std::string& line);

        /// @brief Stops the client and closes the TCP connection.
        ///
        /// Terminates further networking operations and releases
        /// the underlying socket resources.
        void stop();

    private:
        /// @brief Asynchronously reads the next line from the server.
        ///
        /// Processes incoming data and continues receiving messages
        /// while the connection remains active.
        void read_line();

        /// @brief Asynchronously sends the next queued message.
        ///
        /// Retrieves the next message from the outgoing queue and
        /// initiates its transmission through the TCP socket.
        void write_next();

    private:
        /// @brief Indicates whether the client has been stopped.
        ///
        /// Prevents further networking operations after shutdown.
        bool m_stopped = false;

        /// @brief Stores incoming data received from the server.
        ///
        /// Acts as a buffer for assembling messages read from the TCP socket.
        std::string m_input;

        /// @brief Stores messages waiting to be sent to the server.
        ///
        /// Maintains the order of outgoing messages and ensures they
        /// are transmitted sequentially.
        std::deque<std::string> m_outbox;

        /// @brief TCP socket used for communication with the chat server.
        ///
        /// Handles asynchronous read and write operations over
        /// the established connection.
        boost::asio::ip::tcp::socket m_socket;
    };
}