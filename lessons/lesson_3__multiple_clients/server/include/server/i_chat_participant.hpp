#pragma once

#include <string>


namespace server
{

    class IChatParticipant
    {
    public:
        virtual ~IChatParticipant() = default;

        virtual const std::string& name() const = 0;
        virtual void deliver(const std::string& text) = 0;
    };

}
