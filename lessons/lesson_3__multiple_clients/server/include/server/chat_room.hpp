#pragma once


#include "server/i_chat_participant.hpp"
#include "server/i_chat_room.hpp"

#include <memory>
#include <string>
#include <unordered_set>


namespace server
{
    /// @brief Manages connected participants within a chatroom.
    ///
    /// Implements the IChatRoom interface by maintaining a collection
    /// of active participants, managing their membership, checking
    /// nickname availability, and broadcasting messages.
    ///
    /// Each participant is represented by a shared pointer to
    /// IChatParticipant, allowing the chatroom to manage participants
    /// through a common interface.
    class ChatRoom final
        : public IChatRoom
    {
    public:
        /// @brief Adds a participant to the chatroom.
        ///
        /// Registers the provided participant as an active member,
        /// allowing them to receive broadcast messages.
        ///
        /// @param participant Shared pointer to the participant to add.
        void join(const std::shared_ptr<IChatParticipant>& participant) override;

        /// @brief Removes a participant from the chatroom.
        ///
        /// Unregisters the provided participant, preventing them
        /// from receiving further broadcast messages.
        ///
        /// @param participant Shared pointer to the participant to remove.
        void leave(const std::shared_ptr<IChatParticipant>& participant) override;

        /// @brief Checks whether a nickname is already in use.
        ///
        /// Searches the active participants to determine whether
        /// the specified nickname is registered in the chatroom.
        ///
        /// @param name Nickname to check.
        /// @return `true` if the nickname is already taken; otherwise, `false`.
        bool name_taken(const std::string& name) const override;

        /// @brief Broadcasts a message to all connected participants.
        ///
        /// Distributes the provided message to every active participant
        /// currently registered in the chatroom, including the sender
        /// if they are a member.
        ///
        /// @param message Message to broadcast.
        void broadcast(const std::string& message) override;

    private:
        /// @brief Collection of active chatroom participants.
        ///
        /// Stores shared pointers to registered participants, ensuring
        /// that each participant appears only once in the collection.
        ///
        /// Participants remain registered until explicitly removed
        /// through the leave() method.
        std::unordered_set<std::shared_ptr<IChatParticipant>> m_members;
    };
}