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
constexpr std::string_view HOST = "127.0.0.1";


int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Listen only on localhost
    inet_pton(AF_INET, HOST.data(), &server_addr.sin_addr);

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


#### IP, Port and Socket

Some of the biggest concepts in networking in general. I mean, without understanding these 3 meanings, going forward, at least in TCP networking, is going to be really hard. So, what do they mean?

**IP** - stands for Internet Protocol. Most of the connected devices to a network have at least 1 IP address.. Why do devices need it? 

The main idea is that we want to have some ability to distinguish on the network which device is which. Imagine that there is a server (right now do now overthink about what it is, and imagine it like a simple box that clients can connect to) that is an actual chatroom, where clients can connect to and text with each other. If client A sends a message dedicated to client B, how can a server know how to make it so that only the client B would receive and not others? This is where the IP comes into play.

If a client has an IP, now it can be distinguished from that 100 different clients. IP is essentailly like an ID, which allows network to identify devices.

First of all, IP is just a simple number. It's format is this: XXX.XXX.XXX.XXX. A common example of a real IP address: 192.168.1.1. Let's understand, why IP has the format the way it has.

Computers do not think about IP addresses the same way we look at them. For computers, IP is a 32 bit number that is comprised of 0s and 1s. Again, for example, 192.168.1.1 IP address is for one device, 192.168.1.2 is for another device, 200.200.200.200 is for some other device. Because there are 32 bit, in total there are $2^{32} = 4,294,967,295$ different possible IP addresses. 

You might ask "Why only something over 4.2 billion addresses, aren't there more unique devices now?". Well, the answer is yes, but in the 1970s, when computer networking was literally in the child stage, engineers thought that 32 bits will be enough for all the devices that will ever to be. Or they were like "we do not have time to think about it, let's leave for future generations to solve...". Just to make you feel safer, now, there is a 64 bit address, but we will talk about it in the later lessons.

Going back to the IP format. Remember how I said that computers see IP addresses in binary, let's see how it sees some random IP address:

$
    192.168.1.42 = 11000000.10101000.00000001.00101010
$

To better understand it, let's display the same stuff vertically:

$
    \begin{aligned}
        192 & = 11000000 \\
        168 & = 10101000 \\
        1 & = 00000001 \\
        42 & = 00101010
    \end{aligned}
$

PS. to know how to convert binary to decimal, just watch [this simple YouTube short](https://www.youtube.com/shorts/HRfpUqWn7rs). For slightly longer explanations:

* [Binary to Decimal](https://www.youtube.com/watch?v=VLflTjd3lWA&list=PL0o_zxa4K1BXCpQbUdf0htZE8SS0PYjy-)
* [Decimal to Binary](https://www.youtube.com/watch?v=rsxT4FfRBaM)

As you can see, this IP address can be seen as 4 groups (sections) of 8 bit. 8 bits comprise a single **byte** (or octet in networking). So IP address can be seen as a group (section) of 4 bytes.

Why use dots to separate this 32 bit number into 4 different 8 bit numbers. The answer is because it was only desgned this way so that we could easier interpret it. There is no hidden reason behind, only that it is easier for us. For computers, it does not matter, the computers always sees IP addresses as a 32 bit number. The formating is just for us. 

n the previous example I showed how this number 192.168.1.42 looks in binary, but what if every section was all 1s? If a section of 8 bits is all 1s, the number is going to be equal to 255, the largest number that can be comprised of 8 bits. If our 32 bit IP address was all 1s, then the IP would be 255.255.255.255, if it is all 0s, then it is 0.0.0.0.

For now I think that is enough to know about the IP address for this lesson. What I want you to understand is that IP is just a simple number comprised of 32 bits.

**Port** is a logical communication endpoint used by the transport layer (such as TCP or UDP (I will explain what they are later)) to identify a specific program on a service running on a device.

A grea analogy for IPs and ports is imagine a street with apartment buildings. Each building has a unique number assigned to it so that a building could be identified on the stree. Also, each apartment in a given building has a unique number so that it could be identified in that building. In this analogy, the building number is the IP and the apartment number is the port:

| Networking Concept | Real-World Analogy | Purpose |
|--------------------|--------------------|---------|
| **IP Address** | Building address (e.g., 123 Main St.) | Identifies the destination device (building) on the network. |
| **Port** | Apartment number (e.g., Apt. 12) | Identifies the specific application or service (apartment) within the device. |
| **IP + Port** | Full address: 123 Main St., Apt. 12 | Identifies the exact communication endpoint (application on a device). |

There can be a total of 65536 TCP ports and additional 65536 UDP ports (again, we will talk about them later).

We need ports, because a single device might have multiple running applications, so if you want to target a specific application you would need to know a port to it. For example port 10,000 might be an endpoint to service that given 2 numbers outputs their sum, port 10,001 is a service that that deletes all your harddrive data (I hope you will not create a program like that...).

With ports we have a way to reach running programs or services on a device, given that they have are listening (have opened) the corresponding ports.

You might ask "what is an open port?". An open port is a network port which the running application is actively listening for incoming connections. An even simpler explanation is that a port becomes open when a program tells the OS that "I am listening on this port, send incoming connection to me".

Another analogy is that if a house is an IP, then all the ways you can get inside that house are ports (front / back doors, windows, garage and etc.).

```C++
constexpr int PORT = 8080;
constexpr std::string_view HOST = "127.0.0.1";
```

I decided to make them constants for this program, because during it's lifetime, these 2 values are not going to change.


The last concept that is the integral part in a lot of networking is socket.

**Socket** is an object created by the OS that represents one endpoint of a network connection. When I first read this definition, I still was not able to understand the meaning of a socket, but once I read a telephone example I clearly got it. To be honest, this will be a telephone + apartment building example.

The idea is this. Imagine that there are N apartment buildings with M apartments in each building. The are people living in apartments. So each person can be identified with a building ID (street name + building nr.) and an apartment number. There is a special telephone made for these buildings, where instead of normal numbers, you just enter the building ID and an apartment number. So if a person A, that is living in a building 10 and apartment 5, wants to call a person B that is living in building 4 and apartment 17, then the person A will have to dial 4.17 number into his phone. Person B sees that a number 10.5 is calling and answers the phone. This way people can call each other that live in these buildings.

In this example:

| Networking Concept | Apartment Analogy |
| ------------------ | ----------------- |
| **IP Address** | The **building ID** (street + building number). Identifies which building the communication should be delivered to. |
| **Port** | The **apartment number**. Identifies which apartment (application (in this example a person)) inside the building should receive the communication. |
| **Socket** | The **telephone inside the apartment**. It is the communication endpoint used to make and receive calls (network messages). |
| **Application** | The **person living in the apartment**. The application processes the received data and generates responses. |

The communication flow can be visualized as:

```text
Person A
(Building 10, Apartment 5)
        │
    Telephone (Socket)
        │
        ▼
     Dial 4.17
        │
        ▼
Building 4 (IP Address)
        │
Apartment 17 (Port)
        │
Telephone (Socket)
        │
        ▼
Person B (Application)
```

Now we know what all 3 major concepts mean in TCP networking. But just to glide by it one more time:

| Networking Concept | Meaning |
| ------------------ | ------- |
| **IP Address** | A unique 32-bit number assigned to a device on an IPv4 network. It identifies the destination device for network communication. |
| **Port** | A 16-bit number (0–65535) that identifies a specific application or service running on a device. A port is considered **open** when an application is actively listening on it. |
| **Socket** | An operating system object that represents one endpoint of a network communication channel. Applications use sockets to send and receive network data. |

Socket is created using this:

```C++
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
```

Notice that the `server_socket` is an integer. It is because this function returns a descriptor that is essentially an ID to an actual object created somewhere else. For those that have read my opengl lessons, it is very similar to the way that many OpenGL objects or functions are accessable via the integer ID because that ID refers to that to that specific object that is managed internally by OpenGL. Same is here with a `server_socket` that is an ID to an actual object managet internally by OS.

`AF_INET` tells that we want a socket that is of type IPv4. Remember that there is also IPv6.

`SOCK_STREAM` tells that we want a TCP type of communication. Remember that there is also UDP (also, we will talk about it in later lessons).

`0` tells which protocol to use. A protocol is a simple set of rules that 2 devices agree to use when communicating. Different protocols define different rules. For TCP connection we almost always use `0`.


#### Server Logic

```C++
sockaddr_in server_addr{};
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);
```

These lines create a C (language) struct that essentially stores and IPv4 socket address. `sockaddr_in server_addr{};` initializes this struct to default values. `server_addr.sin_family = AF_INET;` tells that this socket address should be IPv4.

`server_addr.sin_port = htons(PORT);` this sets the socket address port. But what does `htons()` do?

`htons()` means **Host TO Network Short**. This function converts 16-bit integer (in C++ it is of type `short`) to a network byte order. Why can't we do simply `server_addr.sin_port = PORT;`? It all comes down to a to this concept of little-endian and big-endian. You can read about this concept [here](https://saliktariq.medium.com/big-endian-and-little-endian-systems-in-c-significance-code-differences-and-performance-487752a20496). In short, the idea behind it is that most of the computers nowadays store bytes using the little-endian format, while network protocols use big-endian format. Why network protocols use big-endian? It was also a design choice in the beginning of network programming. Just know that network protocols use little-endian when defining a port.

These 3 lines are pretty clear what they mean now, but I still had one question when learning this. What is the `sin` prefix in `sin_family` and `sin_port`? I do not know why they needed to add this prefix in the first place, but in is the abbreviation of **socket internet**.

```C++
inet_pton(AF_INET, HOST.data(), &server_addr.sin_addr);
```

This line converts the human readable IP address into the binary format that the OS could understand. So basically this:

```C++
"127.0.0.1"
```

is converted into:

```C++
01111111 00000000 00000000 00000001
```

and we assign the convertes result to `server_addr.sin_addr` variable. `AF_INET` tells that this address should be converted to IPv4 format.


#### Binding

```C++
bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
```

Before I give you the explanation on what is binding, I will take a step back and ask you: what do we have now?

Right now we have a socket descriptor `server_socket` that is just a simple number and `server_addr` struct that is containing necessary configuration information for our socket. The problem is that these 2 variables are not "connected" to each other. As of now we have a socket and a configuration as 2 separate things.

Now, our current socket looks like this:

```
--------------
|            |
| TCP socket |
| No address |
|            |
-------------
```

What we want is this:

```
-----------------
|               |
| TCP socket    |
| IP: 127.0.0.1 |
| Port: 8080    |
|               |
-----------------
```

What `bind()` does is exactly this. It associates the created socket with a defined IP address and a port. It kind of connects `server_addr` info with `server_socket`.

I remember that when I was learning this I had this question: "Why `socket(AF_INET, SOCK_STREAM, 0);` functionality does not allow setting port and ip to the created socket descriptor?". The idea is that because with this line:

```C++
int server_socket = socket(AF_INET, SOCK_STREAM, 0);
```

a socket is created that only knows 3 things:

* Address family: IPv4 (AF_INET)
* Type: TCP (SOCK_STREAM)
* Protocol: TCP

These 3 things are stored somewhere as a single separate concept managed by OS. More precisely OS creates a kernel object that knows only those 3 aforementioned things. You only have a single descriptor ID that associates it with a real kernel object (I will explain what a kernel is soon). But this:

```C++
sockaddr_in server_addr{};
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);
```

is a different concept, that deals with ports and IPs. You combine them with `bind()` to tell the OS to "Associate this with this.", and that's it. Do not overthink too much about this.

Up to this point we have a proper set up socket that is ready to be used. Let's learn how to use it.


#### Listening

```C++
listen(server_socket, 1);
```

This line tells to OS that this socket `server_socket` is a server socket and because of it, start listening for incoming connections. 

Before this line was called, our socket was just a simple TCP endpoint. But this endopoint still does not have a purposeful reason on why it exists. `listen()` is the function that makes our socket capable of listening for incoming clients, meaning that now, our socket is has a purpose. 

This funciton does not create a separate thread or repeatedly check for connection. It tells the kernel that if a new connections arrives on this socket, queue them until my program processes them further.

The kernel object is the real heavy lifter here, because it internally does all of the work like:

* receiving TCP SYN packets (will talk more about them in the future lessons)
* completing the TCP handshake
* maintaining a queue of pending connecitons

Our server program does not have to think about these operations, that is why it is great that we have only a descriptor, you do not have to think about all the under the hood stuff.

What does 1 mean? It is a simple backlog essentially telling how many connections are pending. If it is set to 1, then there can only be a single pending connection. Any more clients trying to connect are not guaranteed to succeed. The outcome depends on the OS and how the TCP is set up on that OS. Any more connections than the backlog might:

* timeout
* connection might be refused
* the client may retry automatically

all of these and others are possible, but it is the OS that decides what's going to happen next with that connection.

Also, notice one thing, `listen()` is not a blocking operation, meaning that the code following this function will be executed right after this function completes it's job. This function completes the job really fast, because it essentially is just a call to the kernel object "put this socket into a listening mode".


#### Accepting a client

```C++
sockaddr_in client_addr{};
socklen_t client_len = sizeof(client_addr);

int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
```

In order to accept a client we first need create an uninitialized `sockaddr_in` struct. Remember that this struct stores IP address (IPv4 or IPv6) and port (everything that we need to know about the client). We need this struct to store information about connected client.

Before accept, `client_address` looks like this:

* IP Address: -
* Port: -

After the `accept()` the `client_address` might look like this:

* IP Address: XXX.XXX.XXX.XXX (remember, that this is actually a 32bit number)
* Port: XXXXX


```C++
socklen_t client_len = sizeof(client_addr);
```

This line, at first, I did not understand. This line tells to the kernel that the buffer size of the `client_addr` is `sizeof(client_addr)`. In C++, `sizeof()` returns the size of an object or a struct in bytes.

We need to know the size because of the way we use `client_addr` in `accept()` function. We need to know the size of this buffer so that the kernel would know how much information it should write into this buffer. 


```C++
int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
```

<!-- First of all, th -->