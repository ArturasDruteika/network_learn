#pragma once


#include <cstddef>
#include <cstdint>
#include <string>


namespace common
{
    /// @brief Default TCP port used by the chat server.
    inline constexpr std::uint16_t default_port = 5555;

    /// @brief Maximum allowed length of a client's nickname.
    inline constexpr std::size_t max_name_length = 24;

    /// @brief Maximum allowed length of a chat message in bytes.
    inline constexpr std::size_t max_message_length = 2048;

    /// @brief Maximum buffer size for incoming protocol lines in bytes.
    ///
    /// Limits the amount of data buffered while receiving newline-delimited
    /// messages from a TCP connection.
    inline constexpr std::size_t max_line_buffer = 4096;

    /// @brief Validates a client's nickname.
    ///
    /// Checks whether the provided nickname satisfies the protocol's
    /// naming requirements, including the maximum allowed length.
    ///
    /// @param name Nickname to validate.
    /// @return `true` if the nickname is valid; otherwise, `false`.
    bool valid_name(const std::string& name);

    /// @brief Removes a trailing carriage return from a protocol line.
    ///
    /// Normalizes incoming text by removing a trailing '\r' character,
    /// allowing compatibility with CRLF line endings.
    ///
    /// Each protocol record ends with '\n', and the first record
    /// sent by a client contains its nickname.
    ///
    /// @param line Protocol line to normalize.
    /// @return The normalized line without a trailing carriage return.
    std::string strip_cr(std::string line);
}