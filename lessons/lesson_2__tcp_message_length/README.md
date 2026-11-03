# Lesson 2: TCP Message Length

## Table of Contents

- [0. Premise](#0-premise)
- [1. Message Length](#1-message-length)
  - [1.1. Problem](#11-problem)
  - [1.2. Solution](#12-solution)
- [2. Message Structure](#2-message-structure)
  - [2.1. Message Length Header](#21-message-length-header)
  - [2.2. Network Byte Order](#22-network-byte-order)
- [3. Implementation](#3-implementation)
  - [3.1. Sending All Bytes](#31-sending-all-bytes)
  - [3.2. Receiving All Bytes](#32-receiving-all-bytes)
  - [3.3. TCP Server](#33-tcp-server)
  - [3.4. TCP Client](#34-tcp-client)
- [4. Testing](#4-testing)
- [5. Conclusion](#5-conclusion)

# 0. Premise

In this lesson, our goal is to learn how to utilate a message length, which is one of the key techniques for efficient and reliable data transfer through TCP sockets. To be honest, this same technique (at least the idea of encoding a message length) is not TCP socket only. A lot of different technologies use message length encoding, for example:

* UDP sockets (which we are going to cover in later lessons)
* ethernet
* bluetooth
* hard drives

Even spacecraft that orbit other planets also use the same principles when they want to send data back to earth. In other words, message length encoding is used almost everywhere where networking or data transfer is applied.


# 1. Message Length

First of all, we need to understand what is a message length? 

**Message Length** is a number that answers the question: how long is this current message? Here are a few examples of what it means:

* "I like cats" - message length: 11
* "I own 3 cats and 1 dog" - message length: 22
* "Ping" - message length: 4

What problem does it solve?


## 1.1. Problem

In lesson 1, in the server side we were doing this:

```C++
char buffer[1024]{};
recv(client_socket, buffer, sizeof(buffer), 0);
```

In a sense, we were reserving a char buffer of size 1024 for the message. Also, the 3-rd argument in the `recv(...)` function tells us that at most we are expecting 1024 at most, but that des not mean "give me exactly 1024 bytes".

Also, in lesson 1 the server, kind of, expected only a single client to connect, but what if there is a situation where a client sends 3 messages, one after the other:

```
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ P │ I │ N │ G │ P │ i │ N │ G │
└───┴───┴───┴───┴───┴───┴───┴───┘
             8 bytes

┌───┬───┬───┬───┐
│ H │ A │ H │ A │
└───┴───┴───┴───┘
     4 bytes


┌───┬───┬───┬───┬───┐
│ H │ E │ L │ L │ O │
└───┴───┴───┴───┴───┘
        5 bytes
```

If the client does the reading after the 3-rd client message has been sent, then the server might receive all 3 messages as a single message, for example:

```
recv() #1 (17 bytes):

┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
│ P │ I │ N │ G │ P │ i │ N │ G │ H │ A │ H │ A │ H │ E │ L │ L │ O │
└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
                             17 bytes
```

It can also receive data in this way:

```
recv() #1 (3 bytes):

┌───┬───┬───┐
│ P │ I │ N │
└───┴───┴───┘

recv() #2 (7 bytes):

┌───┬───┬───┬───┬───┬───┬───┐
│ G │ P │ i │ N │ G │ H │ A │
└───┴───┴───┴───┴───┴───┴───┘

recv() #3 (7 bytes):

┌───┬───┬───┬───┬───┬───┬───┐
│ H │ A │ H │ E │ L │ L │ O │
└───┴───┴───┴───┴───┴───┴───┘
```

In cases like these, we are actually in a problem, because now, when the server does `recv(...)`, it's buffer can return different results:

Do you see the problem now? For all the aforementioned problems and more, we must somehow encode a message length in order to know how many bytes of data a server or a client (later in the future we will talk about this) should expect.


## 1.2. Solution

