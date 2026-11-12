#pragma once

#include <cstddef>
#include <cstdint>
#include <string>


namespace common
{

    inline constexpr std::uint16_t default_port = 5555;
    inline constexpr std::size_t max_name_length = 24;
    inline constexpr std::size_t max_message_length = 2048;
    inline constexpr std::size_t max_line_buffer = 4096;

    // Each record ends with '\n'. The first client record is its nickname.
    bool valid_name(const std::string& name);
    std::string strip_cr(std::string line);

}
