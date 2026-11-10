# Lesson 1: Ping Pong

## Premise

There is a saying that you only truly understand something when you can explain it simply to a child.

That is the idea behind these lessons.

This project is not about building a large networking framework or implementing complicated functionality. Instead, the goal is to understand the fundamentals of networking well enough that we could explain what is happening without hiding behind words such as *socket*, *port*, *kernel*, or *TCP*.

I noticed the value of this approach while writing my OpenGL lessons. Things that initially seemed obvious became surprisingly difficult to explain. Writing the explanations forced me to notice gaps in my own understanding.

I want to use the same approach for networking in C++.

So throughout these lessons, whenever we encounter something unfamiliar, we will try to answer two questions:

1. **What does it do?**
2. **Why does it exist?**

For this first lesson, we will build the smallest TCP program I could think of:

```text
Client                              Server

  │                                   │
  │ ----------- "Ping" -------------> │
  │                                   │
  │ <---------- "Pong" -------------- │
  │                                   │
```

That is all.

Despite the tiny program, implementing it will introduce us to many of the fundamental ideas behind network programming.

---

# Ping Pong

When learning graphics programming, people often begin by rendering a triangle.

For networking, a common first project is a client/server chat application.

I think even that introduces too many things at once.

Instead, we will make a program with exactly one interaction:

```text
Client connects
      │
      ▼
Client sends "Ping"
      │
      ▼
Server receives "Ping"
      │
      ▼
Server sends "Pong"
      │
      ▼
Client receives "Pong"
      │
      ▼
Connection closes
```

There are no loops, multiple clients, threads, asynchronous operations, protocols of our own, or complicated message formats.

The program exists purely to help us understand the building blocks.

---

# Client and Server

For this program we have two roles.

| Role | Responsibility |
|---|---|
| **Server** | Waits for incoming connections and responds to clients |
| **Client** | Initiates a connection to the server and communicates with it |

These are roles, not fundamentally different kinds of programs.

A server is simply a program that offers some service to other programs.

A client is a program that uses that service.

For example:

```text
Weather client ────────> Weather server
                         returns temperature

Game client ───────────> Game server
                         handles game state

Browser ───────────────> Web server
                         returns web content
```

Our service is much simpler:

```text
Ping ────────> Server
     <──────── Pong
```

We therefore need two programs:

```text
server.cpp
client.cpp
```

---

# The Complete Program

Before explaining individual pieces, here is the entire program.

The code intentionally avoids production-level concerns such as retry logic, loops, multiple clients, non-blocking I/O, and extensive error handling.

Those things matter, but they would distract from the concepts we are trying to learn.

## Server

```cpp
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <string_view>
#include <unistd.h>

constexpr int PORT = 8080;
constexpr std::string_view HOST = "127.0.0.1";

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, HOST.data(), &server_addr.sin_addr);

    bind(
        server_socket,
        reinterpret_cast<sockaddr*>(&server_addr),
        sizeof(server_addr)
    );

    listen(server_socket, 1);

    spdlog::info("Waiting for client...");

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(
        server_socket,
        reinterpret_cast<sockaddr*>(&client_addr),
        &client_len
    );

    char buffer[1024]{};

    ssize_t received = recv(
        client_socket,
        buffer,
        sizeof(buffer),
        0
    );

    if (received > 0)
    {
        spdlog::info(
            "Received: {}",
            std::string_view(buffer, static_cast<std::size_t>(received))
        );
    }

    constexpr std::string_view reply = "Pong";

    send(
        client_socket,
        reply.data(),
        reply.size(),
        0
    );

    close(client_socket);
    close(server_socket);

    return 0;
}
```

## Client

```cpp
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <string_view>
#include <unistd.h>

constexpr int SERVER_PORT = 8080;
constexpr std::string_view SERVER_IP = "127.0.0.1";

constexpr int CLIENT_PORT = 9000;
constexpr std::string_view CLIENT_IP = "127.0.0.2";

int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Give the client an explicit local address for this lesson.
    sockaddr_in client_addr{};
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);

    inet_pton(
        AF_INET,
        CLIENT_IP.data(),
        &client_addr.sin_addr
    );

    bind(
        socket_fd,
        reinterpret_cast<sockaddr*>(&client_addr),
        sizeof(client_addr)
    );

    // Address of the server we want to connect to.
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    inet_pton(
        AF_INET,
        SERVER_IP.data(),
        &server_addr.sin_addr
    );

    connect(
        socket_fd,
        reinterpret_cast<sockaddr*>(&server_addr),
        sizeof(server_addr)
    );

    constexpr std::string_view message = "Ping";

    send(
        socket_fd,
        message.data(),
        message.size(),
        0
    );

    char buffer[1024]{};

    ssize_t received = recv(
        socket_fd,
        buffer,
        sizeof(buffer),
        0
    );

    if (received > 0)
    {
        spdlog::info(
            "Server replied: {}",
            std::string_view(buffer, static_cast<std::size_t>(received))
        );
    }

    close(socket_fd);

    return 0;
}
```

> **Important:** TCP is a byte stream. A single `send()` is not generally guaranteed to correspond to a single `recv()`, and `send()` itself may send fewer bytes than requested. Our messages are tiny and the program runs locally, so we keep the example intentionally simple. Later, when we build real message handling, we will deal with partial reads and writes correctly.

The overall program looks like this:

```text
SERVER                                      CLIENT

socket()                                    socket()
   │                                           │
   ▼                                           ▼
bind()                                      bind()
   │                                           │
   ▼                                           │
listen()                                       │
   │                                           │
   ▼                                           ▼
accept()  <──────────── connect() ─────────────┘
   │
   │              TCP connection
   │
   ▼
recv()    <──────────── "Ping" ─────────── send()
   │
   ▼
send()    ───────────── "Pong" ──────────> recv()
   │                                           │
   ▼                                           ▼
close()                                     close()
```

Now we can understand what each part actually means.

---

# Includes

```cpp
#include "spdlog/spdlog.h"
```

`spdlog` is the logging library we use to display information.

For example:

```cpp
spdlog::info("Waiting for client...");
```

---

```cpp
#include <arpa/inet.h>
```

This provides networking utilities such as:

```cpp
inet_pton()
htons()
```

We will use these to prepare IP addresses and ports in the format expected by the socket API.

---

```cpp
#include <unistd.h>
```

On Unix-like systems this provides functions such as:

```cpp
read()
write()
close()
```

For this lesson, the important one is:

```cpp
close()
```

We will later see why sockets work with functions that also operate on files and other OS resources.

---

# IP Addresses, Ports, and Sockets

Before understanding:

```cpp
socket()
bind()
listen()
accept()
connect()
```

we need three concepts:

1. **IP address**
2. **Port**
3. **Socket**

---

# IP Addresses

An IP address identifies a network interface/addressable host location in an IP network so packets can be routed to the correct destination.

For this lesson we use IPv4.

An IPv4 address contains **32 bits**.

A familiar example is:

```text
192.168.1.42
```

The dots are human-readable notation.

The same value can be written in binary:

```text
192      168      1        42

11000000 10101000 00000001 00101010
```

or:

```text
192.168.1.42

11000000.10101000.00000001.00101010
```

Each group contains 8 bits.

```text
8 bits = 1 byte = 1 octet
```

Therefore:

```text
IPv4

4 octets × 8 bits
        =
      32 bits
```

Each octet can represent values from:

```text
0
```

to:

```text
255
```

because:

```text
11111111₂ = 255
```

That gives us addresses ranging syntactically from:

```text
0.0.0.0
```

through:

```text
255.255.255.255
```

There are:

```text
2^32 = 4,294,967,296
```

possible 32-bit bit patterns.

That does **not** mean all 4,294,967,296 values can be assigned freely to devices. IPv4 contains reserved, special-purpose, private, multicast, loopback, and other address ranges.

For now, the important mental model is:

```text
IPv4 address
      │
      ▼
32-bit value used as part of identifying
and routing network communication
```

---

# Loopback

Our programs use:

```cpp
constexpr std::string_view SERVER_IP = "127.0.0.1";
```

`127.0.0.1` is part of the IPv4 **loopback** range.

Loopback traffic stays on the local machine.

So:

```text
127.0.0.1
```

basically means:

```text
this computer
```

for our purposes.

Our client uses:

```text
127.0.0.2
```

which is also inside the IPv4 loopback range on typical Unix-like systems.

That lets us give the client and server visually different local addresses while keeping all communication on the same computer.

---

# Ports

An IP address gets network traffic to the correct host/interface.

But a computer may have many networked applications running simultaneously.

For example:

```text
Computer
│
├── web server
├── game server
├── SSH server
├── database
└── our Ping Pong server
```

The operating system therefore needs another value to help identify the transport-layer endpoint that should receive the traffic.

That value is a **port number**.

A port is a 16-bit unsigned number:

```text
0 - 65535
```

Therefore there are:

```text
65,536
```

possible port numbers for TCP, and UDP has its own 16-bit port-number space.

A useful analogy is an apartment building:

```text
IP address = building address
Port       = apartment number
```

For example:

```text
127.0.0.1:8080
```

can be imagined as:

```text
127.0.0.1     :     8080
     │                 │
     ▼                 ▼
 building          apartment
```

More technically:

```text
IP address + port
        │
        ▼
transport endpoint address
```

Our server uses:

```cpp
constexpr int PORT = 8080;
constexpr std::string_view HOST = "127.0.0.1";
```

so its local endpoint is:

```text
127.0.0.1:8080
```

---

# What Does "Open Port" Mean?

People often say:

> Port 8080 is open.

A port is not literally a physical hole that becomes open.

For TCP, this usually means that the operating system currently has a socket listening for incoming connections on an address involving that port.

Conceptually:

```text
Incoming TCP connection
        │
        │ destination port 8080
        ▼
Operating System
        │
        ▼
Listening socket
127.0.0.1:8080
        │
        ▼
Server application
```

The important point is that ports are part of the networking protocol. They are not physical objects inside the computer.

---

# Sockets

Now we reach the most important abstraction in this lesson.

A **socket** is an operating-system networking object used by a process as an endpoint for network communication.

Our server creates one using:

```cpp
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
```

At first this looks strange.

If `socket()` creates a socket, why does it return an `int`?

```cpp
int server_socket
```

Shouldn't a socket be some giant C++ networking object?

The answer is that the integer is **not the actual socket object**.

On Unix-like systems, it is a **file descriptor**.

A better mental model is:

```text
                         USER SPACE

                    C++ Application
              ┌────────────────────────┐
              │                        │
              │       sock = 3         │
              │          │             │
              └──────────┼─────────────┘
                         │
                  file descriptor
                         │
                         │
═════════════════════════╪═════════════════════════
                  User ↕ Kernel
                         │
                         ▼
                        KERNEL

              ┌────────────────────────┐
              │      Socket Object     │
              ├────────────────────────┤
              │  Protocol state        │
              │  Local/remote address  │
              │  Send/receive buffers  │
              │  Connection state      │
              │  Other metadata        │
              └────────────────────────┘
```

The application keeps a small integer.

The kernel manages the networking object.

---

# `socket()`

Consider:

```cpp
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
```

There are three arguments.

## `AF_INET`

```cpp
AF_INET
```

selects the IPv4 address family.

Later we will encounter:

```cpp
AF_INET6
```

for IPv6.

---

## `SOCK_STREAM`

```cpp
SOCK_STREAM
```

requests a reliable, ordered, stream-oriented socket type.

With the Internet protocol family, this is normally used with TCP.

TCP provides a **byte stream**.

That distinction will become very important later.

TCP does not preserve application message boundaries.

If we do:

```cpp
send(fd, "Hello", 5, 0);
send(fd, "World", 5, 0);
```

the receiver should **not** assume that two calls to `recv()` will necessarily return:

```text
Hello
World
```

It might observe the bytes in different chunks.

The stream itself is logically:

```text
HelloWorld
```

We will handle this properly in later lessons.

---

## Protocol `0`

The final argument is:

```cpp
0
```

This tells the OS to select the default protocol appropriate for the chosen address family and socket type.

With:

```cpp
AF_INET
SOCK_STREAM
```

that normally means TCP.

So:

```cpp
socket(AF_INET, SOCK_STREAM, 0)
```

can roughly be read as:

```text
Create an IPv4 stream socket
using the default matching protocol.
```

---

# Preparing an Address

Creating the socket does not yet tell the kernel which local IP address and port our server should use.

For that we create:

```cpp
sockaddr_in server_addr{};
```

`sockaddr_in` represents an IPv4 socket address.

We then configure it:

```cpp
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);
```

Conceptually:

```text
server_addr

family = IPv4
port   = 8080
IP     = not set yet
```

---

# Why `htons()`?

We write:

```cpp
server_addr.sin_port = htons(PORT);
```

instead of:

```cpp
server_addr.sin_port = PORT;
```

`htons` means:

```text
host to network short
```

It converts a 16-bit value from the machine's host byte order into **network byte order**.

Network byte order is big-endian.

So:

```text
PORT
 │
 │ host representation
 ▼
htons()
 │
 │ network byte order
 ▼
sin_port
```

You do not need to memorize the byte-level details yet.

For now:

```cpp
sin_port = htons(PORT);
```

means:

> Store this port in the byte order expected by the networking API.

---

# Converting the IP Address

Our IP address begins as text:

```cpp
constexpr std::string_view HOST = "127.0.0.1";
```

But `sockaddr_in` does not store the address as the string `"127.0.0.1"`.

We convert the textual representation using:

```cpp
inet_pton(
    AF_INET,
    HOST.data(),
    &server_addr.sin_addr
);
```

`inet_pton` can be read as:

```text
presentation → network
```

Conceptually:

```text
"127.0.0.1"
      │
      ▼
  inet_pton()
      │
      ▼
binary IPv4 address
      │
      ▼
server_addr.sin_addr
```

After this, `server_addr` contains the information necessary to describe:

```text
127.0.0.1:8080
```

---

# `bind()`

We now have two separate things:

```text
Socket object

and

Address:
127.0.0.1:8080
```

The socket has been created, but we have not yet assigned that local address to it.

That is what `bind()` does.

```cpp
bind(
    server_socket,
    reinterpret_cast<sockaddr*>(&server_addr),
    sizeof(server_addr)
);
```

Conceptually:

```text
Before bind()

Socket
┌──────────────────┐
│ IPv4 TCP socket  │
│                  │
│ no chosen local  │
│ address yet      │
└──────────────────┘


Address structure
┌──────────────────┐
│ 127.0.0.1        │
│ 8080             │
└──────────────────┘
```

After:

```cpp
bind(...)
```

we have associated the socket with:

```text
127.0.0.1:8080
```

Conceptually:

```text
Socket
┌──────────────────┐
│ IPv4 TCP socket  │
│                  │
│ local:           │
│ 127.0.0.1:8080   │
└──────────────────┘
```

One subtle point is important:

`bind()` does **not** connect to another computer.

It assigns a **local address** to a socket.

---

# Why Is the Address Separate From `socket()`?

You might wonder why we do this:

```cpp
socket(...)
```

then:

```cpp
bind(...)
```

instead of something like:

```cpp
socket(AF_INET, SOCK_STREAM, "127.0.0.1", 8080);
```

The socket API separates:

```text
Create networking endpoint
```

from:

```text
Assign local address
```

This makes the socket interface flexible.

Also, not every socket needs an explicit `bind()` call from the application.

For example, a normal TCP client can simply call:

```cpp
connect()
```

and the OS can automatically choose an appropriate local IP address and ephemeral local port.

We explicitly bind our client in this lesson only because having known client and server endpoints makes the diagrams easier to understand.

---

# `listen()`

Our server now has:

```text
IPv4 TCP socket
local address: 127.0.0.1:8080
```

But it is not yet a listening socket.

We do:

```cpp
listen(server_socket, 1);
```

`listen()` transitions the socket into a **listening socket**.

Conceptually:

```text
socket()
   │
   ▼
TCP socket
   │
   ▼
bind()
   │
   ▼
TCP socket with local address
   │
   ▼
listen()
   │
   ▼
TCP listening socket
```

The kernel can now handle incoming TCP connection attempts for this listening endpoint.

Importantly:

```cpp
listen(...)
```

does **not** mean:

> Stop the program here and wait for someone.

`listen()` normally returns immediately.

It configures the socket for passive connection handling.

The operation that normally waits for a connection is:

```cpp
accept()
```

---

# What Does the Backlog Mean?

We use:

```cpp
listen(server_socket, 1);
```

The second argument is called the **backlog**.

A simplified mental model is:

> It controls the requested limit for how many completed/pending incoming connections may wait to be accepted.

The exact behavior is operating-system dependent, and kernels may interpret or adjust this value.

So avoid thinking of:

```cpp
listen(fd, 1)
```

as a guarantee that:

> Exactly one client can connect and client number two will immediately fail.

That is too simplistic.

For this lesson, all we need is:

```text
listen(socket, backlog)

                 │
                 ▼

Allow incoming TCP connections
and maintain a queue for connections
waiting for the application to accept them.
```

---

# `accept()`

Now we reach our first important blocking operation:

```cpp
sockaddr_in client_addr{};
socklen_t client_len = sizeof(client_addr);

int client_socket = accept(
    server_socket,
    reinterpret_cast<sockaddr*>(&client_addr),
    &client_len
);
```

If no completed connection is available, a blocking `accept()` waits.

Conceptually:

```text
Server

listen()
   │
   ▼
accept()
   │
   │ waiting...
   │ waiting...
   │ waiting...
   │
   │          Client
   │             │
   │             ▼
   │          connect()
   │             │
   ▼             │
connection available
   │
   ▼
accept() returns
```

---

# Why `client_addr`?

We create:

```cpp
sockaddr_in client_addr{};
```

so the kernel has somewhere to write information about the connected peer.

After `accept()`, it can contain something such as:

```text
IP:   127.0.0.2
Port: 9000
```

---

# Why `client_len`?

We also pass:

```cpp
socklen_t client_len = sizeof(client_addr);
```

and then:

```cpp
&client_len
```

to `accept()`.

The socket API supports multiple address families, whose address structures may have different sizes.

`accept()` therefore receives:

```text
pointer to address memory
+
size of that memory
```

Conceptually:

```text
client_addr
┌─────────────────────────┐
│ memory for peer address │
└─────────────────────────┘
             ▲
             │
       client_len
   tells the kernel how
   much space is available
```

On return, `client_len` is updated to describe the size of the address actually written.

---

# Why Does `accept()` Return Another Socket?

This is one of the most important ideas in TCP server programming.

We already have:

```cpp
server_socket
```

Why do we need:

```cpp
client_socket
```

as well?

Because the listening socket has a different job from a connected socket.

```text
server_socket
     │
     │ listens for NEW connections
     │
     ▼
  accept()
     │
     ▼
client_socket
     │
     │ communicates with ONE connection
     ▼
 send()/recv()
```

The listening socket remains available for accepting additional connections.

Each accepted TCP connection receives its own connected socket.

For our tiny program we only accept one connection, but the distinction is fundamental.

---

# The Three Socket Objects

Once the connection is established, our example contains **three socket objects across the two processes**.

```text
CLIENT PROCESS                         SERVER PROCESS


                                      ┌──────────────────────┐
                                      │   server_socket      │
                                      │                      │
                                      │ 127.0.0.1:8080       │
                                      │                      │
                                      │ LISTENING            │
                                      └──────────┬───────────┘
                                                 │
                                              accept()
                                                 │
                                                 ▼

┌──────────────────────┐              ┌──────────────────────┐
│      socket_fd       │              │    client_socket     │
│                      │              │                      │
│ local:               │              │ local:               │
│ 127.0.0.2:9000       │              │ 127.0.0.1:8080       │
│                      │              │                      │
│ remote:              │              │ remote:              │
│ 127.0.0.1:8080       │              │ 127.0.0.2:9000       │
└──────────┬───────────┘              └──────────┬───────────┘
           │                                     │
           │                                     │
           └════════ TCP CONNECTION ═════════════┘
```

So:

| Socket | Process | Purpose |
|---|---|---|
| `socket_fd` | Client | Client side of the TCP connection |
| `server_socket` | Server | Listens for new connections |
| `client_socket` | Server | Server side of this particular TCP connection |

Only two of those are the endpoints of the established connection:

```text
client socket                       server connected socket

127.0.0.2:9000  <====== TCP ======> 127.0.0.1:8080
```

The listening socket is separate.

---

# `recv()`

After `accept()` returns, the server has a connected socket:

```cpp
client_socket
```

We can receive data from it.

```cpp
char buffer[1024]{};

ssize_t received = recv(
    client_socket,
    buffer,
    sizeof(buffer),
    0
);
```

The buffer begins as:

```text
buffer

┌────┬────┬────┬────┬────┬─────────┐
│  0 │  0 │  0 │  0 │  0 │   ...   │
└────┴────┴────┴────┴────┴─────────┘
```

If the client sends:

```text
Ping
```

then `recv()` may copy those bytes into our buffer:

```text
buffer

┌─────┬─────┬─────┬─────┬────┬─────────┐
│ 'P' │ 'i' │ 'n' │ 'g' │  0 │   ...   │
└─────┴─────┴─────┴─────┴────┴─────────┘
```

More importantly, `recv()` returns the number of bytes actually received.

For example:

```cpp
received == 4
```

We should use that value rather than assuming the buffer contains a null-terminated C string.

---

# What Does `recv()` Actually Mean?

A useful misconception to remove immediately is:

```text
recv() = start receiving network traffic
```

That is not how it works.

Network traffic can arrive while your program is doing something completely different.

The kernel's networking subsystem processes incoming traffic and maintains receive state/buffers for sockets.

So a better mental model is:

```text
Network
   │
   ▼
Network interface
   │
   ▼
Kernel networking stack
   │
   ▼
Socket receive buffer/state
   │
   │ recv()
   ▼
Your application's buffer
```

`recv()` is closer to:

> Give my process some bytes currently available for this socket. If none are available and this is a blocking socket, wait until the operation can make progress or another relevant condition occurs.

---

# Why Can `recv()` Block?

Suppose we call:

```cpp
recv(socket_fd, buffer, sizeof(buffer), 0);
```

but no data is currently available.

For a normal blocking socket, the calling thread may be put to sleep while it waits.

```text
Application thread
       │
       ▼
    recv()
       │
       ▼
Receive data available?
       │
   ┌───┴───┐
   │       │
  yes      no
   │       │
   │       ▼
   │    wait/sleep
   │       │
   │   data arrives
   │       │
   └───┬───┘
       ▼
recv() returns
```

The CPU does not need to waste time executing:

```cpp
while (nothing_received)
{
    // repeatedly check forever
}
```

The operating system can suspend the thread and schedule other work.

---

# User Space, Kernel, and Hardware

At this point, understanding the basic relationship between our application and the operating system becomes useful.

A simplified computer model looks like this:

```text
┌──────────────────────────────────────────┐
│                USER SPACE                │
│                                          │
│   Browser    Editor    Your C++ Program  │
│                                          │
├──────────────────────────────────────────┤
│                  KERNEL                  │
│                                          │
│   Processes      Memory management       │
│   Filesystems    Networking              │
│   Drivers        Socket subsystem        │
│                                          │
├──────────────────────────────────────────┤
│                 HARDWARE                 │
│                                          │
│   CPU      RAM      SSD      Network NIC │
│                                          │
└──────────────────────────────────────────┘
```

This is deliberately simplified.

---

# User Mode and Kernel Mode

Ordinary applications normally execute in **user mode**.

The kernel executes privileged code in **kernel mode**.

The distinction is enforced by the CPU together with memory-management mechanisms configured by the operating system.

A user-space process can normally:

```text
✓ perform arithmetic
✓ execute branches
✓ call ordinary functions
✓ use CPU registers
✓ read its permitted memory
✓ write its permitted memory
```

But it cannot freely:

```text
✗ access kernel memory
✗ access another process's memory
✗ modify page tables arbitrarily
✗ execute privileged CPU instructions
✗ directly control protected hardware however it wants
```

A useful picture is:

```text
                     USER MODE

              ┌────────────────────┐
              │  Your C++ process  │
              │                    │
              │  arithmetic     ✓  │
              │  registers      ✓  │
              │  own memory     ✓  │
              │  branches       ✓  │
              │  functions      ✓  │
              └─────────┬──────────┘
                        │
                        │ system calls
                        ▼
════════════════════════════════════════════
                        │
                        ▼
                   KERNEL MODE

              ┌────────────────────┐
              │       Kernel       │
              │                    │
              │  networking       │
              │  filesystems      │
              │  drivers          │
              │  process control  │
              │  memory mgmt      │
              └────────────────────┘
```

This is why ordinary arithmetic does not require asking the kernel:

```cpp
int c = a + b;
```

The CPU can execute that instruction directly in user mode.

But operations involving protected operating-system resources require controlled transitions into the kernel.

---

# System Calls and Networking

Functions such as:

```cpp
socket()
bind()
listen()
accept()
connect()
send()
recv()
close()
```

ultimately request services from the operating system.

Conceptually:

```text
Your C++ program
       │
       │ socket()
       │ bind()
       │ listen()
       │ accept()
       │ connect()
       │ send()
       │ recv()
       │ close()
       ▼
Operating-system interface
       │
       ▼
Kernel
       │
       ▼
Networking subsystem
       │
       ▼
Drivers
       │
       ▼
Network hardware
```

There may be library wrappers and implementation details between your C++ call and the kernel, so do not interpret this diagram as exact machine instructions.

It is a mental model.

The important idea is:

> Your application requests networking operations; the kernel manages the protected networking state and hardware interactions.

---

# File Descriptors

We can finally explain why:

```cpp
socket()
```

returns an integer.

On Unix-like systems, a process refers to many open resources using small integers called **file descriptors**.

Conceptually, a process has a file-descriptor table resembling:

```text
File descriptor table

0 ──────> standard input
1 ──────> standard output
2 ──────> standard error
3 ──────> socket
4 ──────> file
5 ──────> another socket
```

If:

```cpp
int socket_fd = socket(...);
```

returns:

```text
3
```

then `3` is the process's handle for that open socket.

It is not the socket's global ID.

Another process can also have a descriptor `3`, and its `3` may refer to something completely different.

So a more accurate definition is:

> A file descriptor is a small non-negative integer used by a Unix-like process to refer to an open file description/resource managed through the operating system.

For this lesson, the simpler mental model is:

```text
fd
 │
 ▼
small process-local handle
 │
 ▼
kernel-managed open resource
```

---

# Standard File Descriptors

Unix-like processes conventionally begin with three standard file descriptors:

```text
0 ──────> stdin
1 ──────> stdout
2 ──────> stderr
```

`unistd.h` provides symbolic names:

```cpp
STDIN_FILENO
STDOUT_FILENO
STDERR_FILENO
```

where:

```text
STDIN_FILENO  == 0
STDOUT_FILENO == 1
STDERR_FILENO == 2
```

For example:

```cpp
#include <unistd.h>

int main()
{
    const char message[] = "Hello!\n";

    write(
        STDOUT_FILENO,
        message,
        sizeof(message) - 1
    );
}
```

This writes bytes to standard output.

We can similarly write to standard error:

```cpp
#include <unistd.h>

int main()
{
    const char error[] = "Something went wrong!\n";

    write(
        STDERR_FILENO,
        error,
        sizeof(error) - 1
    );
}
```

Or read from standard input:

```cpp
#include <unistd.h>

int main()
{
    char buffer[100];

    ssize_t bytes = read(
        STDIN_FILENO,
        buffer,
        sizeof(buffer)
    );
}
```

Notice how similar this looks to operating on a socket:

```cpp
read(socket_fd, buffer, sizeof(buffer));
```

That is one of the powerful ideas behind Unix file descriptors.

Many different kinds of I/O resources can be accessed through a common descriptor-based interface.

---

# Redirection

Standard descriptors do not necessarily refer to a terminal.

For example:

```bash
./program < input.txt
```

redirects standard input.

```bash
./program > output.txt
```

redirects standard output.

```bash
./program 2> errors.txt
```

redirects standard error.

So:

```text
stdout
```

does not mean:

```text
the terminal
```

It means:

```text
file descriptor 1
```

and that descriptor may currently refer to a terminal, file, pipe, or another compatible resource.

---

# Back to Our Socket

Now this line should make much more sense:

```cpp
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
```

Conceptually:

```text
Application
    │
    │ socket(...)
    ▼
Kernel creates socket state
    │
    ▼
File descriptor allocated
    │
    ▼
Application receives integer
```

For example:

```text
                         USER SPACE

                    C++ Application
              ┌────────────────────────┐
              │                        │
              │       sock = 3         │
              │                        │
              └──────────┬─────────────┘
                         │
                   descriptor 3
                         │
═════════════════════════╪═════════════════════════
                         │
                         ▼
                        KERNEL

              ┌────────────────────────┐
              │      Socket State      │
              │                        │
              │  protocol information  │
              │  addresses             │
              │  buffers               │
              │  connection state      │
              │  ...                   │
              └────────────────────────┘
```

The descriptor is the application's reference to the resource.

---

# Where Does Received Data Wait?

Suppose data arrives from the network before our application calls:

```cpp
recv()
```

The bytes do not need to disappear.

Very roughly:

```text
Network
   │
   ▼
NIC
   │
   ▼
Kernel networking stack
   │
   ▼
Socket receive buffer
┌───────────────────────┐
│ P │ i │ n │ g │ ...   │
└───────────────────────┘
   │
   │ recv()
   ▼
User-space buffer
```

The exact kernel implementation is more complicated, but this is a useful mental model.

---

# How Does the OS Know Which Socket Gets Traffic?

Incoming TCP traffic contains identifying information such as:

```text
source IP
source port
destination IP
destination port
protocol
```

For an established TCP connection, the connection is commonly described by:

```text
local IP
local port
remote IP
remote port
```

together with the protocol.

For our connection:

```text
Client

local:  127.0.0.2:9000
remote: 127.0.0.1:8080
```

and:

```text
Server connected socket

local:  127.0.0.1:8080
remote: 127.0.0.2:9000
```

The kernel maintains networking state that allows incoming traffic to be associated with the appropriate socket.

That is why your application does not need to inspect every packet arriving at the network interface.

---

# `send()`

The server replies with:

```cpp
constexpr std::string_view reply = "Pong";

send(
    client_socket,
    reply.data(),
    reply.size(),
    0
);
```

Conceptually:

```text
Application memory
"Pong"
   │
   │ send()
   ▼
Kernel socket state/buffers
   │
   ▼
TCP/IP stack
   │
   ▼
Network interface
   │
   ▼
Network
```

`send()` does not mean:

> These exact bytes have already physically reached the other application.

It gives data to the socket/networking subsystem for transmission and returns according to the socket's state and the operation's semantics.

Also remember:

> `send()` can return after accepting fewer bytes than requested.

A robust application must check its return value and continue sending if necessary.

We deliberately postpone that logic because it would obscure the basic flow of this first program.

---

# `close()`

At the end we do:

```cpp
close(client_socket);
close(server_socket);
```

`close()` is not specific to sockets.

It closes a **file descriptor**.

Conceptually:

```text
Before:

fd 3 ─────> socket


close(3)


After:

fd 3 ─────> no longer refers to that resource
```

The descriptor number can later be reused.

For example:

```text
socket() returns 3
close(3)

later...

open("file.txt") returns 3
```

So after:

```cpp
close(socket_fd);
```

you must treat `socket_fd` as invalid unless you deliberately assign a new valid descriptor to it.

---

# Does `close()` Immediately Destroy the Socket?

Not necessarily in the simplistic sense.

A file descriptor is one reference to an underlying open resource.

There can be situations involving duplicated descriptors, inherited descriptors, or other references.

The underlying resource can remain alive while references to it still exist.

For a simple program like ours, where the descriptor is the last reference, closing the connected TCP socket causes the kernel to perform the appropriate TCP connection-closing behavior.

The exact TCP shutdown sequence depends on connection state and circumstances.

For now, the correct mental model is:

```text
close(fd)
   │
   ▼
Process releases this descriptor
   │
   ▼
If it was the last relevant reference,
the kernel can clean up the underlying resource
and perform protocol-specific teardown.
```

We will study TCP connection termination separately rather than pretending it is always simply:

```text
FIN → ACK → destroyed
```

because real TCP teardown is more nuanced.

---

# Quick Socket Questions

At this point we can answer several questions that initially seem mysterious.

## Why does `socket()` return an integer?

Because Unix-like systems expose the newly created socket to the process through a file descriptor.

```text
socket()
   │
   ▼
kernel-managed socket
   │
   ▼
file descriptor
   │
   ▼
int
```

---

## Why can `recv()` block?

Because the application asked for incoming data and none may currently be available.

With a blocking socket, the operating system can suspend the calling thread until the operation can continue.

---

## Where does received data wait?

Typically in kernel-managed networking/socket buffers and related networking state until the application consumes it.

---

## Can data arrive before I call `recv()`?

Yes.

The kernel's networking subsystem handles incoming traffic independently of whether your application is currently inside `recv()`.

`recv()` asks for data associated with the socket; it does not turn the network interface on.

---

## How does the kernel know which socket gets incoming traffic?

It uses protocol and endpoint/connection information from the incoming traffic together with its internal networking tables and socket state.

---

## What does `close()` close?

The file descriptor.

If that releases the final reference to the underlying socket resource, the kernel can clean up that resource and perform the appropriate protocol-specific teardown.

---

# Server Flow So Far

We can now understand almost the entire server.

```text
socket()
   │
   │ create an IPv4 TCP socket
   ▼
bind()
   │
   │ assign local address
   │ 127.0.0.1:8080
   ▼
listen()
   │
   │ make it a listening socket
   ▼
accept()
   │
   │ wait for / obtain one connection
   ▼
connected socket
   │
   ▼
recv()
   │
   │ receive "Ping"
   ▼
send()
   │
   │ send "Pong"
   ▼
close()
```

Now we only need to understand the client.

---

# Client

The client starts the same way:

```cpp
int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
```

Again:

```text
Client process
     │
     ▼
socket()
     │
     ▼
IPv4 TCP socket
```

---

# Client Address

In many normal TCP clients, we would **not** explicitly bind a local IP and port.

The OS could choose them automatically when we call:

```cpp
connect()
```

For this lesson, however, we explicitly choose:

```cpp
constexpr int CLIENT_PORT = 9000;
constexpr std::string_view CLIENT_IP = "127.0.0.2";
```

because it makes the connection easier to visualize.

We create the address:

```cpp
sockaddr_in client_addr{};

client_addr.sin_family = AF_INET;
client_addr.sin_port = htons(CLIENT_PORT);

inet_pton(
    AF_INET,
    CLIENT_IP.data(),
    &client_addr.sin_addr
);
```

and bind it:

```cpp
bind(
    socket_fd,
    reinterpret_cast<sockaddr*>(&client_addr),
    sizeof(client_addr)
);
```

Now the client socket has the local endpoint:

```text
127.0.0.2:9000
```

Again, this explicit client-side `bind()` is primarily educational.

A more typical client would let the operating system select an available ephemeral port.

---

# Server Address on the Client

The client also needs to know **where it wants to connect**.

So we create another address:

```cpp
sockaddr_in server_addr{};

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(SERVER_PORT);

inet_pton(
    AF_INET,
    SERVER_IP.data(),
    &server_addr.sin_addr
);
```

This describes:

```text
127.0.0.1:8080
```

Notice the difference:

```text
client_addr
    │
    ▼
Where THIS socket lives locally


server_addr
    │
    ▼
Where THIS socket wants to connect
```

---

# `connect()`

Finally:

```cpp
connect(
    socket_fd,
    reinterpret_cast<sockaddr*>(&server_addr),
    sizeof(server_addr)
);
```

This asks the operating system to establish a TCP connection to:

```text
127.0.0.1:8080
```

For a normal blocking TCP socket, `connect()` does not simply fire a packet and immediately return successfully.

The kernel performs the TCP connection-establishment process.

At a high level:

```text
CLIENT                                  SERVER

connect()
   │
   │                                      listening
   │                                         │
   ├──────── connection attempt ────────────>│
   │                                         │
   │<──── TCP establishment traffic ────────>│
   │                                         │
   ▼                                         ▼
connect() returns                    connection queued
successfully                                │
                                             ▼
                                          accept()
                                             │
                                             ▼
                                    connected socket
```

We will study the TCP three-way handshake in detail later.

For this lesson, all we need is:

```text
connect()
    =
attempt to establish a TCP connection
to the specified remote endpoint
```

---

# `connect()` and `accept()`

These two functions are closely related, but they run in different processes.

```text
SERVER                               CLIENT

listen()
   │
   ▼
accept()  <─────────────── connect()
   │
   │
   ▼
server receives
connected socket
```

The client initiates the connection with:

```cpp
connect()
```

The server obtains an application-visible connected socket with:

```cpp
accept()
```

A subtle but important point:

The kernel may complete the TCP handshake and queue the established connection before the server application actually calls or returns from `accept()`.

So `accept()` should not be thought of as:

> Perform the TCP handshake yourself.

It is closer to:

> Give my process one established connection waiting for this listening socket.

---

# Sending Ping

Once `connect()` succeeds, the client can send data:

```cpp
constexpr std::string_view message = "Ping";

send(
    socket_fd,
    message.data(),
    message.size(),
    0
);
```

Conceptually:

```text
CLIENT                         SERVER

"Ping"
   │
   ▼
send()
   │
   └───────────────> recv()
```

---

# Receiving Pong

The client then waits for the response:

```cpp
char buffer[1024]{};

ssize_t received = recv(
    socket_fd,
    buffer,
    sizeof(buffer),
    0
);
```

The server sends:

```text
Pong
```

and the client receives those bytes:

```text
CLIENT                         SERVER

recv() <─────────────── send()
   │                     "Pong"
   ▼
"Pong"
```

Finally:

```cpp
close(socket_fd);
```

releases the client's descriptor.

---

# The Entire Connection

We can now understand the whole program.

```text
                 SERVER                         CLIENT

                   │                              │
                   ▼                              ▼
                socket()                       socket()
                   │                              │
                   ▼                              ▼
                 bind()                         bind()
                   │                              │
                   ▼                              │
                listen()                          │
                   │                              │
                   ▼                              ▼
                accept() <────── connect() ───────┘
                   │
                   │       TCP connection
                   │       established
                   │
                   ▼
                 recv() <─────── "Ping" ────── send()
                   │
                   ▼
                 send() ──────── "Pong" ─────> recv()
                   │                              │
                   ▼                              ▼
                close()                        close()
```

---

# What Exists After the Connection Is Established?

This is worth repeating because it is one of the easiest things to misunderstand.

## Client Process

```text
socket_fd
│
├── local:  127.0.0.2:9000
└── remote: 127.0.0.1:8080
```

## Server Process

Listening socket:

```text
server_socket
│
└── local: 127.0.0.1:8080
```

Connected socket:

```text
client_socket
│
├── local:  127.0.0.1:8080
└── remote: 127.0.0.2:9000
```

Therefore:

```text
CLIENT PROCESS                         SERVER PROCESS

 socket_fd                              server_socket
     │                                      │
     │                                      └── listening
     │
     │                                  client_socket
     │                                      │
     └══════════ TCP CONNECTION ════════════┘
```

There are:

```text
3 socket objects total
```

but the established TCP connection has:

```text
2 connected endpoints
```

one on each side.

---

# Important Distinction: Listening Socket vs Connected Socket

The listening socket:

```cpp
server_socket
```

does not represent one particular client connection.

Its job is:

```text
wait for new connections
```

The socket returned by:

```cpp
accept()
```

represents one specific established connection.

Its job is:

```text
send and receive data
```

That is why a real server can eventually look like:

```cpp
while (true)
{
    int client = accept(server_socket, ...);

    // communicate with this client
}
```

The listening socket survives and continues accepting connections.

Each connection receives another socket.

We will build that kind of server later.

---

# One More Important TCP Detail

Our program gives the impression that TCP sends messages:

```text
"Ping"
"Pong"
```

Technically, TCP does not know that those are messages.

TCP sees bytes.

The client writes:

```text
P i n g
```

The server receives a stream containing those bytes.

If an application performs:

```cpp
send(fd, "ABC", 3, 0);
send(fd, "DEF", 3, 0);
```

TCP conceptually provides:

```text
A B C D E F
```

The receiver might observe them through `recv()` calls as:

```text
ABCDEF
```

or:

```text
AB
CDEF
```

or another valid grouping.

The byte order is preserved, but the application must define its own message boundaries.

That will become extremely important once we start building real protocols.

For our four-byte Ping/Pong example, we intentionally ignore this complexity for now.

---

# What We Learned

From an application that only sends:

```text
Ping
```

and receives:

```text
Pong
```

we already encountered most of the basic building blocks of TCP socket programming.

## IP Address

For IPv4:

```text
32-bit address
```

used by the IP layer as part of addressing and routing network traffic.

Example:

```text
127.0.0.1
```

---

## Port

A 16-bit transport-layer number:

```text
0 - 65535
```

used by protocols such as TCP and UDP as part of identifying communication endpoints.

Example:

```text
8080
```

---

## Endpoint Address

```text
IP + port
```

Example:

```text
127.0.0.1:8080
```

---

## Socket

A kernel-managed networking object used by a process as a communication endpoint.

---

## File Descriptor

A small process-local integer used on Unix-like systems to refer to an open resource such as a socket.

```text
3 ─────> socket
```

---

## `socket()`

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

Creates a socket and returns a file descriptor.

---

## `bind()`

```cpp
bind(...);
```

Associates a socket with a local address.

```text
127.0.0.1:8080
```

---

## `listen()`

```cpp
listen(...);
```

Turns a suitable TCP socket into a listening socket that can receive incoming connection requests.

---

## `accept()`

```cpp
accept(...);
```

Obtains one established connection from a listening socket.

On a blocking socket, it waits if no connection is currently available.

It returns a **new connected socket**.

---

## `connect()`

```cpp
connect(...);
```

Initiates a connection to a remote endpoint.

---

## `send()`

```cpp
send(...);
```

Writes bytes to a connected socket's outgoing byte stream.

---

## `recv()`

```cpp
recv(...);
```

Reads bytes from a connected socket.

With a blocking socket, it may wait when no data is currently available.

---

## `close()`

```cpp
close(...);
```

Closes the process's file descriptor.

For sockets, releasing the final reference allows the kernel to clean up the socket and perform the appropriate protocol-specific teardown.

---

# Final Mental Model

If there is only one diagram to remember from this lesson, remember this one:

```text
                          USER SPACE

                 ┌─────────────────────┐
                 │   Client Program    │
                 │                     │
                 │ socket_fd = 3       │
                 └──────────┬──────────┘
                            │
                            │ system calls
                            │
════════════════════════════╪════════════════════════════
                            │
                            ▼
                           KERNEL

                 ┌─────────────────────┐
                 │    Socket State     │
                 │                     │
                 │ local address       │
                 │ remote address      │
                 │ TCP state           │
                 │ receive state       │
                 │ send state          │
                 │ ...                 │
                 └──────────┬──────────┘
                            │
                            ▼
                  Kernel networking stack
                            │
                            ▼
                       NIC / Network
                            │
                            ▼
                         Network
```

And for the connection itself:

```text
CLIENT PROCESS                                  SERVER PROCESS

┌─────────────────────┐                  ┌─────────────────────┐
│      socket_fd      │                  │    server_socket    │
│                     │                  │                     │
│ 127.0.0.2:9000      │                  │ 127.0.0.1:8080      │
│                     │                  │                     │
│ CONNECTED           │                  │ LISTENING           │
└──────────┬──────────┘                  └──────────┬──────────┘
           │                                        │
           │                                     accept()
           │                                        │
           │                                        ▼
           │                             ┌─────────────────────┐
           │                             │    client_socket    │
           │                             │                     │
           │                             │ 127.0.0.1:8080      │
           │                             │                     │
           │                             │ CONNECTED           │
           │                             └──────────┬──────────┘
           │                                        │
           │                                        │
           └══════════════ TCP CONNECTION ══════════┘
```

The client and server applications do not manually control packets, network cards, TCP state machines, retransmissions, routing, or most of the other machinery involved.

They work through sockets.

The kernel handles the lower-level networking machinery.

Our application mostly sees:

```text
socket()
bind()
listen()
accept()
connect()
send()
recv()
close()
```

That small interface hides an enormous amount of complexity.

And that is exactly why sockets are useful.

---

# Final Program Flow

```text
SERVER                                              CLIENT

socket(AF_INET, SOCK_STREAM, 0)             socket(AF_INET, SOCK_STREAM, 0)
        │                                            │
        ▼                                            ▼
bind(127.0.0.1:8080)                        bind(127.0.0.2:9000)
        │                                            │
        ▼                                            │
listen()                                             │
        │                                            │
        ▼                                            ▼
accept()  <─────────────────────────────── connect(127.0.0.1:8080)
        │                                            │
        │                                            │
        │           CONNECTION ESTABLISHED           │
        │                                            │
        ▼                                            ▼
recv()   <──────────────── "Ping" ─────────────── send()
        │                                            │
        ▼                                            │
send()   ──────────────── "Pong" ───────────────> recv()
        │                                            │
        ▼                                            ▼
close(client_socket)                            close(socket_fd)
        │
        ▼
close(server_socket)
```

That is our first TCP application.

The program itself is tiny.

But underneath those few function calls we now have a mental model for:

```text
IPv4
ports
endpoints
sockets
file descriptors
user mode
kernel mode
socket buffers
blocking operations
listening sockets
connected sockets
bind()
listen()
accept()
connect()
send()
recv()
close()
```

In the next lessons, we can start adding complexity without treating these concepts as magic.