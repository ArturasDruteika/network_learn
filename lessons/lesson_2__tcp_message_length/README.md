# Lesson 2: TCP Message Length

## Premise

In this lesson, our goal is to learn how to utilate a message length, which is one of the key techniques for efficient and reliable data transfer through TCP sockets. To be honest, this same technique (at least the idea of encoding a message length) is not TCP socket only. A lot of different technologies use message length encoding, for example:

* UDP sockets (which we are going to cover in later lessons)
* ethernet
* bluetooth
* hard drives

Even spacecraft that orbit other planets also use the same principles when they want to send data back to earth. In other words, message length encoding is used almost everywhere where networking or data transfer is applied.


# Message Length

First of all, we need to understand what is a message length? 

**Message Length** is a number that answers the question: how long is this current message? Here are a few examples of what it means:

* "I like cats" - message length: 11
* "I own 3 cats and 1 dog" - message length: 22
* "Ping" - message length: 4

What problem does it solve?


### The Problem

In lesson 1, in the server side we were doing this:

```C++
char buffer[1024]{};
recv(client_socket, buffer, sizeof(buffer), 0);
```

In a sense, we were reserving a char buffer of size 1024 for the message. Also, the 3-rd argument in the `recv(...)` function tells us that at most we are expecting 1024 at most, but that des not mean "give me exactly 1024 bytes".

Also, in lesson 1 the server, kind of, expected only a single client to connect, but what if there is a situation where 2 client connect and both at the same time do:

```C++
send(socket_fd, message, strlen(message) + 1, 0);
```

In this case, we are actually in a problem, because now, when the server does `recv(...)`, it's buffer is going to be looking like:

```
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ P │ I │ N │ G │ P │ i │ N │ G │
└───┴───┴───┴───┴───┴───┴───┴───┘
             8 bytes
```

What if a single client sends some random data:

```
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ a │ H │ 4 │ 8 │ Q │ , │ 5 │ G │
└───┴───┴───┴───┴───┴───┴───┴───┘
             8 bytes
```

Since our server does not hold any logic on how to process the incoming messages, it actually is not a problem, but what if the serveronly responded when it receved exactly this:

```
┌───┬───┬───┬───┐
│ P │ I │ N │ G │
└───┴───┴───┴───┘
     4 bytes
```