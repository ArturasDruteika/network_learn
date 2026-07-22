# Lesson 1: Ping Pong


## Premise 

I have never been a person that wanted to spend time teaching other how stuff works, but this year I fully realized the saying "You are only an expect in a subject if you can explain it in a way that a child could understand it".

I wanted to say this, that this project is not meant to be a big project with complex functionalities. This project is more about understanding the basics of networking so that we could explain it in a way that children could understand what we are doing.

Also, I decided to make these lessons, because I have another lessons that I started to do before networking. Those lessons are about rendering using OpenGL. While writing down those lessons, I observed the fact that what I thought was pretty "easy" is not that easy once I tried to explain it to others. That meant that I was not a specialist of that subject. But once I wrote down the first few lessons, I clearly noticed that I had better understand how basic things worked in redering.

With these lesson about rendering, I intend to teach myself about how networking using C++ works. With the word "myself" I imply also people that will be reading these lessons. 

In the end, I hope that I and you will be able to understand the building blocks of networking so that we could all explain it to the little kids.


## Ping Pong

In rendering there is a saying that everyone should start learning it by understanding and rendering a single triangle on a screen. In networking, most of the internet says that people should start learning it by creating a simple server / client program where a client can chat with a server and receive messages from server. 

This is all great, but for me, I found that to be too hard for me. That is why I have decided to go even deeper. This is where the ping pong test came to my mind. 

It is a very simple program which goal is to ping a server and receive the response if that server is running.

This program is pretty linear, with no loops or anything hard to understand. The only hard parts in this lesson are the basic concepts of networking which, currently, we do not understand.

So, without further adu, let's dive into the code and understand everything to become specialists.


## Program

In networking, almost always there are 2 types of roles for the programs:

1. **Server** - program that is dealing with client requests
2. **Client** - program that sends requests to the server

The idea that you need to have a server and a client, but, there can be many different servers and many different client implementations. For example, one server might only be responsible for temperature, meaning that if a client connects, this server immediately sends the current weather temperature, another server might be a chatroom where clients can connect and communicate with each other. Same idea goes with clients also. One client could work as some sort of a notification medium which can ping the temperature server to receive the temperature, the other client want to connect and communicate with other clients so it will connect to the chatroom server and etc..

In order to have a valid ping pong logic, we need to have 2 programs: srver and client.


### Note!!!

Before we begin, I just wanted to express, that some lines of the code will not be explained due to the fact that the line is clear or intuitive.

Server code:

```C++
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <unistd.h>


constexpr int PORT = 8080;


int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 1);

    spdlog::info("Waiting for client...");

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);

    char buffer[1024]{};

    recv(client_socket, buffer, sizeof(buffer), 0);

    spdlog::info("Received: {}", buffer);

    const char* reply = "Pong";

    send(client_socket, reply, strlen(reply) + 1, 0);

    close(client_socket);
    close(server_socket);

    return 0;
}
```

Client code:

```C++
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <string_view>
#include <unistd.h>


constexpr int PORT = 8080;
constexpr std::string_view HOST = "127.0.0.1";


int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, HOST.data(), &server_addr.sin_addr);

    connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr));

    const char* message = "Ping";

    send(socket_fd, message, strlen(message) + 1, 0);

    char buffer[1024]{};

    recv(socket_fd, buffer, sizeof(buffer), 0);

    spdlog::info("Server replied: {}", buffer);

    close(socket_fd);

    return 0;
}
```

If you struggle to visually understand this program, I hope this image will help you:

![](assets/ping_pong_test.png)


### Server code

Let's first analyze the server code, because without it, the concept of clients kind of does not make any sense.


#### Includes

`#include "spdlog/spdlog.h"` a simple logging tool. We will never use `std::cout` since spdlog is many times better.

`#include <arpa/inet.h>` library provides internet address manipulation functions. What these are will be explained in later sections.

`#include <unistd.h>` library that provides access to OS services such as:

* File operations:
    
    * `read()`
    * `write()`
    * `close()`

* Process management:
    
    * `forc()`
    * `exec()`
    * `getpid()`

* Sleeping:

    * `sleep()`
    * `usleep()`

* Pipes:

    * `pipe()`

* File descritor operations:

    * `dup()`
    * `dup2()`

Whether we will need these operations later, I do not know, but I still, I felt the need to write them down...


#### IP and Port

Some of the biggest concepts in networking in general. I mean, without understanding these 2 meanings, going forward, at least in TCP networking, is going to be really hard. So, what do they mean?

**IP** - stands for Internet Protocol. It is an unique address like assigned to every device that is connected to any network. Why do devices need it? 

The main idea is that we want to have some ability to distinguish on the network which device is which. Imagine that there is a server (right now do now overthink about what it is, and imagine it like a simple box that clients can connect to) that is an actual chatroom, where clients can connect to and text with each other. If client A sends a message dedicated to client B, how can a server know how to make it so that only the client B would receive and not others? This is where the IP comes into play.

If a client has an IP, now it can be distinguished from that 100 different clients. IP is essentailly like an ID, which allows network to identify devices.

