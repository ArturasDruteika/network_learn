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
- [3. Conclusion](#3-conclusion)
- [4. Hexadecimal Numbers](#4-hexadecimal-numbers)

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

In this section, our goal is to see, how do we encode a message length on the client side and then how do we process the message length on the server side.


## 2.1. Encoding Message Length

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

What does this function do? Actually, it is pretty simple. The function takes `const std::string& message` and outputs a vector `std::vector<uint8_t>` (which is of size 2) which is the encoding of the message length. 

For example:

**For example:**

Original message: `"I want these cookies!!!"` (23 bytes)

    ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
    │ I │   │ w │ a │ n │ t │   │ t │ h │ e │ s │ e │   │ c │ o │ o │ k │ i │ e │ s │ ! │ ! │ ! │
    └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
                                              23 bytes

Encoded message:

    ┌──────┬──────┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
    │ 0x00 │ 0x17 │ I │   │ w │ a │ n │ t │   │ t │ h │ e │ s │ e │   │ c │ o │ o │ k │ i │ e │ s │ ! │ ! │ ! │
    └──────┴──────┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
    └─────────────┘ └───────────────────────────────────────────────────────────────────────────────────────────┘
       2 bytes                                       23 bytes
   (message length)                               (message data)

                                            25 bytes total

Do not forget that each entry is actually a binary number taking up exactly 1 byte, so to be fair, this message looks like this:

    Encoded message (binary representation):

    Encoded message (25 bytes):

    ┌──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┐
    │ 00000000 │ 00010111 │ 01001001 │ 00100000 │ 01110111 │ 01100001 │ 01101110 │ 01110100 │ 00100000 │ 01110100 │ 01101000 │ 01100101 │ 01110011 │ 01100101 │ 00100000 │ 01100011 │ 01101111 │ 01101111 │ 01101011 │ 01101001 │ 01100101 │ 01110011 │ 00100001 │ 00100001 │ 00100001 │
    └──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┘
    │ 0        │ 23       │ I        │ Space    │ w        │ a        │ n        │ t        │ Space    │ t        │ h        │ e        │ s        │ e        │ Space    │ c        │ o        │ o        │ k        │ i        │ e        │ s        │ !        │ !        │ !        │
    └─────────────────────┘ └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
            2 bytes                                                                                                      23 bytes

                                                                                                                Total: 25 bytes (200 bits)

    Message length:  2 bytes
    Message data:   23 bytes
    ────────────────────────
    Total:          25 bytes (200 bits)

The first 2 bytes encode the message length (23) in big-endian format, followed by the actual message data. 

Notice, how we encode message length using hexadecimal nubers. More about them can be read [here](#5-hexadecimal-numbers).

We use hexadecimal because:

- It makes binary values easier to read.
- I just wanted to introduce hexadecimal numbers.

I think that everything in this function is pretty easy to understand except these lines:

```C++
    uint16_t message_size = static_cast<uint16_t>(message_length);

    message_bytes.push_back(static_cast<uint8_t>(message_size >> 8));
    message_bytes.push_back(static_cast<uint8_t>(message_size));
```

Let's analyze these a bit more. As seen in the previous sketch, we need to transform the original message `[I][][c][o][n][t][a][i][n][][-][][2][0][][b][y][t][e][s]` so that this message, at the beginning, for the first 2 bytes, would have the message length (in hexadecimal) encoded. The outer product of this message should look like: `[0x00][0x14][I][][c][o][n][t][a][i][n][][-][][2][0][][b][y][t][e][s]`.

First of all, we create a variable that in C++ is 2 bytes by itself: `uint16_t`, and we cast to it the `message_length` that we extracted before. We need it, because this is the actual size that we are going to use for message length encoding.

You may notice that the `std::vector<uint8_t> encode_message(const std::string& message)` function returns a vector of `uint8_t`, so where does the `uint16_t message_size` dissapear? 

This is exactly where the bottom 2 lines come into play. These 2 lines, with the help of a little bit of [bit shifting](https://www.geeksforgeeks.org/cpp/left-shift-right-shift-operators-c-cpp/) transform the `uint16_t message_size` into 2 `uint8_t` numbers and append them to the encoded message vector. This is how these 2 lines work:

    16-bit integer (uint16_t message_size):
    00000000 00010100

    Into two separate bytes:
    00000000 - 1-st byte
    00010100 - 2-nd byte

Perfect, we understood how to encode a message. Let's understand how a server side (or the one that receives the messages) processes the message when it receives it.


## 2.2. Reading the Message Length

```C++
/**
 * @brief Reads the message length from a TCP socket.
 *
 * Reads exactly two bytes from the socket, representing an unsigned
 * 16-bit message length in network byte order (big-endian).
 *
 * Since TCP is a byte stream, multiple recv() calls may be required
 * to receive the complete length prefix.
 *
 * @param socket_fd_client File descriptor of the connected client socket.
 *
 * @return Message length in bytes, converted to host byte order.
 *
 * @throws std::runtime_error If the client disconnects before both bytes are received.
 * @throws std::system_error If recv() fails for a reason other than EINTR.
 */
uint16_t read_message_size(int socket_fd_client)
{
    // Our protocol:
    //
    // [ 2 bytes: message size ][ message bytes... ]
    //
    // The message size is stored in network byte order (big-endian).
    //
    // First receive exactly two bytes containing the message size.
    uint16_t message_size_network = 0;

    size_t total_size_received = 0;

    // TCP is a byte stream. One recv() is NOT guaranteed to return
    // both bytes, so keep receiving until we have exactly two bytes.
    while (total_size_received < BYTES_MESSAGE_LENGTH)
    {
        ssize_t received = recv(socket_fd_client,
                                reinterpret_cast<uint8_t*>(&message_size_network) + total_size_received,
                                BYTES_MESSAGE_LENGTH - total_size_received,
                                0);

        if (received < 0 && errno == EINTR)
            continue;

        if (received == 0)
            throw std::runtime_error("Client disconnected before the complete message length was received");

        if (received < 0)
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to receive message length");

        total_size_received += static_cast<size_t>(received);
    }

    // Convert the message size from network byte order (big-endian)
    // to the host's native byte order.
    uint16_t message_size = ntohs(message_size_network);

    return message_size;
}
```

The goal of this function is simple. This function gets a socket, where some data, with unknown size, is buffered, and spits out the message length that is not larger a maximum value possible using 2 bytes (`uint16_t`).

Let's analyze line by line what each line does.

```C++
uint16_t message_size_network = 0;
```

This is the variable that is going to store the actual message length.

```C++
size_t total_size_received = 0;
```

Variable that stores total received bytes. Remember that a single call of `recv(...)` is not guaranteed to return everything we need, so the server (or the receiver) needs to track how many total bytes it has received.

```C++
while (total_size_received < BYTES_MESSAGE_LENGTH)
{
    ...
}
```

Main while loop that iterates not more than `BYTES_MESSAGE_LENGTH` number of iterations. We need this loop, because the server needs to make sure that it receives `BYTES_MESSAGE_LENGTH` number of bytes. Those `BYTES_MESSAGE_LENGTH` number of bytes will tell what is the message length.

```C++
ssize_t received = recv(socket_fd_client,
                        reinterpret_cast<uint8_t*>(&message_size_network) + total_size_received,
                        BYTES_MESSAGE_LENGTH - total_size_received,
                        0);

...

total_size_received += static_cast<size_t>(received);
```

The server reads the data from the socket and stores the number of received bytes to the `received` variable. Do not forget that `recv(...)` returns the number of received bytes.

The 2 important arguments that we pass are these:

- `reinterpret_cast<uint8_t*>(&message_size_network) + total_size_received`: pointer to the first byte of the `message_size_network` + whatever `total_size_received` has. So if `total_size_received` is 0, then `reinterpret_cast<uint8_t*>(&message_size_network)` points to the first byte of `message_size_network`, if `total_size_received` is 1, then `reinterpret_cast<uint8_t*>(&message_size_network)` points to the second byte of `message_size_network`.
- `BYTES_MESSAGE_LENGTH - total_size_received`: read up to that many bytes from the socket.

```C++
total_size_received += static_cast<size_t>(received);
```

If, by any chance `recv(...)` returned only a single byte, then the condition in the while loop is not met, so we are going to have another iteration. Because of it, we need to update the `total_size_received` that tracks how many bytes we have already received.

This scheme summerizes what this while loop, whose goal is to tell the message length, goal is:

                    START
                      |
                      v
          total_size_received = 0
                      |
                      v
       +-----------------------------+
       | total_size_received < 2 ?   |<---------------+
       +-----------------------------+                |
              |             |                         |
             YES            NO                        |
              |             |                         |
              v             v                         |
       +----------------+  FINISH                     |
       |     recv()     |                             |
       |                |                             |
       | Read up to     |                             |
       | 2 - total_size |                             |
       | bytes          |                             |
       +----------------+                             |
              |                                       |
              v                                       |
       +-----------------------------+                |
       | total_size_received         |                |
       |     += received             |                |
       +-----------------------------+                |
              |                                       |
              +---------------------------------------+

Last, but not least, we need to understand how we handle problems during reading from socket. We need these 3 `if` statements, because `recv(...)` does not always return the number of bytes received successfully. `recv(...)` can, essentially, return these values:

| Return value | Meaning | What your code should do |
|--------------|---------|--------------------------|
| `> 0` | Successfully received bytes | Add to `total_size_received` |
| `0` | Client closed its sending side (EOF) | Throw an exception |
| `-1` | An error occurred | Retry or throw, depending on the error |

For this reason, we need these 3 `if` statements:

```C++
if (received < 0 && errno == EINTR)
    continue;

if (received == 0)
    throw std::runtime_error("Client disconnected before the complete message length was received");

if (received < 0)
    throw std::system_error(errno,
                            std::generic_category(),
                            "Failed to receive message length");
```

These `if` statements, in chronological order, do:

- The first `if` statement checks the condition when the `recv(...)` is waiting for bytes and a interrupt signal is received. This is why the `&& errno == EINTR` exists. Imagine a server started this loop and during the waiting (do not forget that if there are no bytes to be read in the socket, `recv(...)` becomes a blocking operation) period an external interrupt signal was received.

- The second `if` statement checks if the client has disconnected once `recv(...)` was invoked. When a client disconnects, then `recv(...)` return `0`.

- The third if statement handles all other problems when receiving bytes from the socket.

Great, now we know how to encode bytes length and how to read them from the socket.


## 2.3. Sending and Receiving the Message

```C++
// client.cpp
std::vector<uint8_t> message_bytes = encode_message(message_1);


// Send the encoded bytes through the TCP connection.
//
// TCP may send fewer bytes than requested in a single send().
// Keep sending until the entire encoded message has been sent.
size_t total_sent = 0;

while (total_sent < message_bytes.size())
{
    ssize_t sent = send(socket_fd_client,
                        message_bytes.data() + total_sent,
                        message_bytes.size() - total_sent,
                        0);

    if (sent < 0 && errno == EINTR)
        continue;

    if (sent <= 0)
    {
        spdlog::error("Failed to send message");
        close(socket_fd_client);
        return 1;
    }

    // Remember how many bytes we have sent so far.
    total_sent += static_cast<size_t>(sent);
}

// server.cpp
std::string message_received;
message_received.resize(message_size);

size_t total_received = 0;

// TCP is a byte stream. One recv() is NOT guaranteed to return
// the entire message, so keep receiving until we have all bytes.
while (total_received < message_received.size())
{
    ssize_t received = recv(socket_fd_client,
                            // Continue writing after the bytes we already received.
                            message_received.data() + total_received,
                            // Ask only for the number of bytes still missing.
                            message_received.size() - total_received,
                            0);

    if (received < 0 && errno == EINTR)
        continue;

    if (received <= 0)
    {
        spdlog::error("Connection closed while receiving message");

        close(socket_fd_client);
        close(socket_fd_server);

        return 1;
    }

    // Remember how many bytes of the message we have so far.
    total_received += static_cast<size_t>(received);
}
```

If you notice, these 2 loops that are responsible for sending and receiving the data are actually very similar to the way the server was reading the message length. And, to be honest, this is a common practice how people are designing their programs when dealing with sockets. 

In a sense, the design follows these steps:

1. Define how many bytes we need to transfer (send or receive)
2. Initialize a counter to track the transfer process
3. Use a `while` loop to continue transfering bytes
4. Call `send(...)` or `recv(...)` (depending on the role) to transfer the remaining bytes
5. Handle possible errors
6. Update the counter after a successful operation
7. Repeat until all bytes have been transfered

The important thing to understand is that TCP is a byte stream and not a message-based protocol. TCP guarantees that bytes are delivered in order, but does not guarantee that a single `send(...)` corresponds to a single `recv(...)`.

This is the entire byte transfer scheme that happens most of the time:

    CLIENT                                     SERVER
    |                                          |
    |  1. Encode message                       |
    |     [Length: 2 bytes][Message: N bytes]  |
    |                                          |
    |  2. Initialize sending counter           |
    |     total_sent = 0                       |
    |                                          |
    |  3. Send encoded bytes                   |
    |     (repeat send() until all bytes       |
    |      have been sent)                     |
    | ---------------------------------------> |
    |                                          |
    |                             4. Read 2 bytes
    |                                (message length)
    |                                          |
    |                             5. Decode length
    |                                (network to host)
    |                                          |
    |                             6. Read N bytes
    |                                (message data)
    |                                          |
    |                             7. Process message
    |                                          |
    |                             8. Encode response
    |                                [Length][Response]
    |                                          |
    |                             9. Initialize sending
    |                                counter
    |                                total_sent = 0
    |                                          |
    |                            10. Send response
    |                                (repeat send()
    |                                 until all bytes
    |                                 have been sent)
    | <--------------------------------------- |
    |                                          |
    | 11. Read 2 bytes                         |
    |     (response length)                    |
    |                                          |
    | 12. Decode response length               |
    |     (network to host)                    |
    |                                          |
    | 13. Read N bytes                         |
    |     (response data)                      |
    |                                          |
    | 14. Process response                     |
    |                                          |
    | 15. Close connection (if finished)       |
    | ---------------------------------------> |
    |                                          |
    |                            16. Detect EOF
    |                                and close socket
    |                                          |
    v                                          v
    END                                        END


# 3. Conclusion

In this lesson, we learnt how to implement a simple length-prefixed protocol over TCP.

We covered how to encode a message, using a predefined number of bytes and how to correctly read the message based on the decoded length of the message. 

These simple concepts are the building blocks of designing network systems. Almost all the networking protocols use the message length idea, it just might be implemented in other way. Encoding and reading the message length is the way to tell the system where a message begins and where it ends.


# 4. Hexadecimal Numbers

In C++, a hexadecimal number is denoted when `0x` is added before the number. A few examples:

    C++ Hexadecimal Table

    | Decimal | Hexadecimal |
    |---------|-------------|
    | 0       | 0x0         |
    | 1       | 0x1         |
    | 5       | 0x5         |
    | 9       | 0x9         |
    | 10      | 0xA         |
    | 11      | 0xB         |
    | 12      | 0xC         |
    | 13      | 0xD         |
    | 14      | 0xE         |
    | 15      | 0xF         |
    | 16      | 0x10        |
    | 32      | 0x20        |
    | 100     | 0x64        |
    | 255     | 0xFF        |


```cpp
#include <iostream>


int main() {
    int num = 255;

    std::cout << dec << num << std::endl; // 255
    std::cout << hex << num << std::endl; // ff

    return 0;
}
```

Note:

- `hex` - Displays numbers in hexadecimal.
- `dec` - Displays numbers in decimal.
- `0x` - Prefix for hexadecimal literals in C++.
- Hexadecimal uses digits `0-9` and letters `A-F` (base 16).
