# Lesson 2: TCP Message Length

## Table of Contents

- [0. Premise](#0-premise)
- [1. Message Length](#1-message-length)
    - [1.1. Problem](#11-problem)
    - [1.2. Solution](#12-solution)
- [2. Implementation](#2-implementation)
    - [2.1. Sending All Bytes](#21-sending-all-bytes)
    - [2.2. Receiving All Bytes](#22-receiving-all-bytes)
    - [2.3. TCP Server](#23-tcp-server)
    - [2.4. TCP Client](#24-tcp-client)
- [3. Testing](#3-testing)
- [4. Conclusion](#4-conclusion)

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

If the client does the reading after the 3-rd client message has been sent, then we are not certain in what order the server is going to receive the data once. The server might receive all 3 messages as a single message, for example:

    recv() #1 (17 bytes):

    ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
    │ P │ I │ N │ G │ P │ i │ N │ G │ H │ A │ H │ A │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
                                17 bytes

It can also receive data in this way:

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

Do you see the problem now? For all the aforementioned problems and more, we must somehow encode a message length in order to know how many bytes of data a server or a client (later in the future we will talk about this) should expect.


## 1.2. Solution

One of the best ways to solve this problem is to encode the message length into the message itself. Before continuing, let's understand why encoding a message length is useful.

By encoding a message length we tell can easily tell the receiver (the one thatcalls `recv()`) how many bytes should be read from the bytes stream. If we know the message length, we can construct different messages from the data stream. 

To add on why encoding mesasge length is great:

- **Efficiant memory allocation**. By knowing the exact message length we can assign that number as the buffer size that will store that message. (In lesson 1 we had to predefine a buffer size of 1024 bytes, which was a huge resource throw away, since the sent message took less than 10 bytes)
- **Detecting incomplete messages**. If we know the message length, we can also know if the message is corrupt or not complete if the received message is less than the actual size. Without knowing the actual message length, we cannot be sure if the received message is full. This is especially critical once we start downloading / uploading files, because the file size after the `send()` or `recv()` operation must occupy the same amount of space.
- **Encode messages of different sizes**. We are not bound by some predefined message length. Each message can have it's own size.

How to encode a message length in the message that is about to be sent?

Let's take this message:

    ┌───┬───┬───┬───┬───┐
    │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┘
           5 bytes

This message is 5 bytes long. What if we could extend this message at the front by adding a predefined number of bytes that would store a number defining a message length like this:

    ┌───┬───┬───┬───┬───┬───┐
    │ 5 │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┴───┘
    └───┘ └─────────────────┘
    1 byte    5 bytes
    (message  (message data)
    length)   

           9 bytes (total)


That is one of the most common and simple ways to encode a message length.

One thing to pay attention to is that each block in the message is of size 1 byte. So by this logic, the number 5 is also represented in the form of 1 byte. If you remember the binary, you should know that a number 5 looks like:

    00000101

Remember that a maximum positive integer (`uint_8`) number that a 1 byte can represent is 255. That means that a message larger than that cannot be encoded if we use a single byte for message length.

To solve this, we can say that 4 bytes will be used for encoding a message length. This way a message could look like this:

    ┌───┬───┬───┬───┬───┬───┬───┬───┬───┐
    │ 0 │ 0 │ 0 │ 5 │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┴───┴───┴───┴───┘
                    8 bytes

Since we use 4 bytes, we can encode a message that is this big 4,294,967,295.

The only predefined rule we need to follow is that the sender and the receiver have to know beforehand the number of bytes that will comprise a message length. For example if a client sends a message where 2 bytes encode a message length, then a server also has to know that the first 2 bytes will tell the message length.

Now we have a basic understanding how to encode a number representing a message length at the beginning of the message itself.


# 2. Implementation

Let's first analyze the client part.

```C++
// Encode a message using our simple protocol:
//
// [ 2 bytes: message size ][ message bytes... ]
//
// Example for "Hello":
//
// [0x00][0x05][H][e][l][l][o]
//
// Because the message size is stored in two bytes (uint16_t),
// our protocol can represent message sizes from 0 to 65535.
//
// The message size is encoded in network byte order (big-endian).
std::vector<uint8_t> encode_message(const std::string& message)
{
    size_t message_length = message.size();

    // Make sure the message length fits into two bytes.
    if (message_length > UINT16_MAX)
    {
        throw std::runtime_error("Message size over the limit. Maximum possible message size: " +
                                 std::to_string(UINT16_MAX) +
                                 ", current message size: " +
                                 std::to_string(message_length));
    }

    std::vector<uint8_t> message_bytes;

    // Reserve memory for:
    //
    //   2 bytes for the message size
    //   +
    //   N bytes for the actual message
    //
    // reserve() allocates capacity but does not add elements.
    message_bytes.reserve(BYTES_MESSAGE_LENGTH + message_length);

    // Convert the message size to a 16-bit unsigned integer.
    uint16_t message_size = static_cast<uint16_t>(message_length);

    // Store the message size in network byte order (big-endian).
    //
    // First byte:  most significant 8 bits.
    // Second byte: least significant 8 bits.
    //
    // Example:
    //
    // Message size = 300 (0x012C)
    //
    // First byte  = 0x01
    // Second byte = 0x2C
    message_bytes.push_back(static_cast<uint8_t>((message_size >> 8) & 0xFF));
    message_bytes.push_back(static_cast<uint8_t>(message_size & 0xFF));

    // Append the actual message bytes after the size bytes.
    message_bytes.insert(message_bytes.end(),
                         message.begin(),
                         message.end());

    return message_bytes;
}
```

What does this function do? Actually, it is pretty simple. The function takes `const std::string& message` and outputs a vector `std::vector<uint8_t>` (which is of size 2) which is the encoding of the message length
