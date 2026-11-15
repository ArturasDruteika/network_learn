#include "server/chat_room.hpp"

#include "spdlog/spdlog.h"


namespace server
{

    void ChatRoom::join(const std::shared_ptr<IChatParticipant>& participant)
    {
        m_members.insert(participant);
    }

    void ChatRoom::leave(const std::shared_ptr<IChatParticipant>& participant)
    {
        m_members.erase(participant);
    }

    bool ChatRoom::username_taken(const std::string& username) const
    {
        for (const std::shared_ptr<server::IChatParticipant>& member : m_members)
        {
            if (member->username() == username)
                return true;
        }

        return false;
    }

    void ChatRoom::broadcast(const std::string& message)
    {
        spdlog::info("{}", message);

        for (const std::shared_ptr<server::IChatParticipant>& member : m_members)
            member->deliver(message);
    }
}