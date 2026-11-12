#include "common/protocol.hpp"

#include <algorithm>
#include <cctype>
#include <utility>


namespace common
{

    bool valid_name(const std::string& name)
    {
        if (name.empty() || name.size() > max_name_length)
            return false;

        return std::all_of(name.begin(),
                           name.end(),
                           [](unsigned char character)
                           {
                               return std::isalnum(character) || character == '_' || character == '-';
                           });
    }


    std::string strip_cr(std::string line)
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        return line;
    }

}
