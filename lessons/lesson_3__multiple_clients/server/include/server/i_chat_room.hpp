#pragma once

#include <memory>
#include <string>


namespace server
{

    class IChatParticipant;

    class IChatRoom
    {
    public:
        virtual ~IChatRoom() = default;

        virtual void join(const std::shared_ptr<IChatParticipant>& participant) = 0;
        virtual void leave(const std::shared_ptr<IChatParticipant>& participant) = 0;
        virtual bool name_taken(const std::string& name) const = 0;
        virtual void broadcast(const std::string& message) = 0;
    };

}
