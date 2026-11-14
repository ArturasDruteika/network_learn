#pragma once


#include "i_chat_participant.hpp"

#include <memory>
#include <string>


namespace server
{
    /// @brief Defines the interface for managing a chatroom.
    ///
    /// Provides operations for registering and removing participants,
    /// checking nickname availability, and broadcasting messages
    /// to connected clients.
    ///
    /// Concrete implementations are responsible for managing
    /// participant membership and message distribution.
    class IChatRoom
    {
    public:
        /// @brief Virtual destructor.
        virtual ~IChatRoom() = default;

        /// @brief Adds a participant to the chatroom.
        ///
        /// Registers the provided participant as an active member,
        /// allowing them to receive broadcast messages.
        ///
        /// @param participant Shared pointer to the participant to add.
        virtual void join(const std::shared_ptr<IChatParticipant>& participant) = 0;

        /// @brief Removes a participant from the chatroom.
        ///
        /// Unregisters the provided participant, preventing them
        /// from receiving further broadcast messages.
        ///
        /// @param participant Shared pointer to the participant to remove.
        virtual void leave(const std::shared_ptr<IChatParticipant>& participant) = 0;

        /// @brief Checks whether a nickname is already in use.
        ///
        /// Determines whether the specified nickname belongs to
        /// an active participant in the chatroom.
        ///
        /// @param name Nickname to check.
        /// @return `true` if the nickname is already taken; otherwise, `false`.
        virtual bool name_taken(const std::string& name) const = 0;

        /// @brief Broadcasts a message to all connected participants.
        ///
        /// Distributes the provided message to every active participant
        /// currently registered in the chatroom, including the sender
        /// if they are a member.
        ///
        /// @param message Message to broadcast.
        virtual void broadcast(const std::string& message) = 0;
    };
}