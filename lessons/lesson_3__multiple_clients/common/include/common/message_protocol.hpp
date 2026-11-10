#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <vector>


class MessageProtocol
{
public:
    static constexpr std::size_t LENGTH_BYTES = 2;
    static constexpr std::size_t MAX_MESSAGE_SIZE = 65535;

    // Convert a payload into:
    //
    // [ 2 bytes: message size ][ message bytes... ]
    static std::vector<uint8_t> encode(std::string_view message);

    // Decode the two-byte big-endian length prefix.
    static uint16_t decode_length(const uint8_t* bytes);
};