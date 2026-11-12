#pragma once

#include "server/i_chat_participant.hpp"
#include "server/i_chat_room.hpp"

#include <memory>
#include <string>
#include <unordered_set>


namespace server
{

    class ChatRoom final 
        : public IChatRoom
    {
    public:
        void join(const std::shared_ptr<IChatParticipant>& participant) override;
        void leave(const std::shared_ptr<IChatParticipant>& participant) override;
        bool name_taken(const std::string& name) const override;
        void broadcast(const std::string& message) override;

    private:
        std::unordered_set<std::shared_ptr<IChatParticipant>> m_members;
    };

}
