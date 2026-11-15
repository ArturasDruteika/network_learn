#include "server/chat_room.hpp"

#include <iostream>


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

    bool ChatRoom::name_taken(const std::string& name) const
    {
        for (const std::shared_ptr<server::IChatParticipant>& member : m_members)
        {
            if (member->name() == name)
                return true;
        }

        return false;
    }

    void ChatRoom::broadcast(const std::string& message)
    {
        std::cout << message << '\n';

        for (const std::shared_ptr<server::IChatParticipant>& member : m_members)
            member->deliver(message);
    }
}
