#pragma once


class Socket
{
public:
    explicit Socket(int fd = -1) noexcept;

    ~Socket();

    // A socket must have only one owner.
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Ownership can be transferred.
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    int fd() const noexcept;
    bool valid() const noexcept;
    void close() noexcept;

private:
    int m_fd;
};