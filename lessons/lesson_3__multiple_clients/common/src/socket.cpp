#include "common/socket.hpp"

#include <unistd.h>
#include <utility>


Socket::Socket(int fd) noexcept
    : m_fd(fd)
{
}


Socket::~Socket()
{
    close();
}


Socket::Socket(Socket&& other) noexcept
    : m_fd(std::exchange(other.m_fd, -1))
{
}


Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        close();
        m_fd = std::exchange(other.m_fd, -1);
    }

    return *this;
}


int Socket::fd() const noexcept
{
    return m_fd;
}


bool Socket::valid() const noexcept
{
    return m_fd >= 0;
}


void Socket::close() noexcept
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}