#pragma once


#include <string>


namespace server
{
    /// @brief Defines the interface for a chatroom participant.
    ///
    /// Represents a client connected to the chatroom and provides
    /// operations for retrieving the participant's nickname and
    /// delivering messages.
    ///
    /// Concrete implementations are responsible for managing
    /// the underlying communication mechanism.
    class IChatParticipant
    {
    public:
        /// @brief Virtual destructor.
        virtual ~IChatParticipant() = default;

        /// @brief Retrieves the participant's nickname.
        ///
        /// @return Constant reference to the participant's nickname.
        virtual const std::string& name() const = 0;

        /// @brief Delivers a message to the participant.
        ///
        /// Forwards the provided message to the participant through
        /// the underlying communication mechanism.
        ///
        /// @param text Message to deliver to the participant.
        virtual void deliver(const std::string& text) = 0;
    };
}