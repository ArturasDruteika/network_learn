#include "common/message_protocol.hpp"


#include <stdexcept>
#include <string>


std::vector<uint8_t> MessageProtocol::encode(std::string_view message)
{
    if (message.size() > MAX_MESSAGE_SIZE)
    {
        throw std::runtime_error("Message exceeds maximum size of " +
                                 std::to_string(MAX_MESSAGE_SIZE) + " bytes");
    }

    const auto size = static_cast<uint16_t>(message.size());

    std::vector<uint8_t> bytes;
    bytes.reserve(LENGTH_BYTES + message.size());

    // Network byte order (big-endian).
    bytes.push_back(static_cast<uint8_t>(size >> 8));
    bytes.push_back(static_cast<uint8_t>(size & 0xFF));

    bytes.insert(bytes.end(),
                 message.begin(),
                 message.end());

    return bytes;
}


uint16_t MessageProtocol::decode_length(const uint8_t* bytes)
{
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8) |
                                  static_cast<uint16_t>(bytes[1]));
}