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

In this lesson, our goal is to learn how to use message lengths, which is one of the key techniques for efficient and reliable data transfer through TCP sockets. This technique (or at least the idea of encoding a message length) is not limited to TCP sockets. Many different technologies use message length encoding, including:

* UDP sockets (which we are going to cover in later lessons)
* Ethernet
* Bluetooth
* hard drives

Even spacecraft orbiting other planets use the same principles when they want to send data back to Earth. In other words, message length encoding is used almost everywhere networking or data transfer is involved.


# 1. Message Length

First, we need to understand what a message length is. 

**Message length** is a number that answers the question: How long is this message? Here are a few examples of what it means:

* "I like cats" - message length: 11
* "I own 3 cats and 1 dog" - message length: 22
* "Ping" - message length: 4

What problem does it solve?


## 1.1. Problem

In Lesson 1, we did the following on the server side:

```C++
char buffer[1024]{};
recv(client_socket, buffer, sizeof(buffer), 0);
```

In a sense, we were allocating a character buffer with a size of 1,024 bytes for the message. Additionally, the third argument of the `recv(...)` function specifies that we can receive at most 1,024 bytes, but that does not mean "give me exactly 1024 bytes".

Additionally, in Lesson 1, the server was designed to handle only a single client connection. But what happens if a client sends three messages, one after another?

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

If the server reads the buffered data from the socket after the client has sent the third message, we cannot be certain that calling `recv(...)` three times will return three separate messages with the same boundaries as the original messages. In fact, we might receive all the data in a single buffer that looks like this:

    recv() #1 (17 bytes):

    ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
    │ P │ I │ N │ G │ P │ i │ N │ G │ H │ A │ H │ A │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
                                17 bytes

The server might also receive the data in the following way:

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

Do you see the problem now? To solve these problems, we need to encode the message length in order to know how many bytes of data a server or client should expect. We will discuss this in more detail later.


## 1.2. Solution

One of the best ways to solve this problem is to encode the message length into the message itself. Before continuing, let's understand why encoding a message length is useful.

By encoding the message length, we can easily tell the receiver (the one that calls `recv()`) how many bytes should be read from the byte stream. If we know the message length, we can reconstruct individual messages from the data stream. 

There are several additional benefits to encoding message lengths:

- **Efficient memory allocation**. By knowing the exact message length, we can use that number to determine the buffer size needed to store the message. (In lesson 1 we had to predefine a buffer size of 1024 bytes, which resulted in unnecessary memory usage, since the sent message took less than 10 bytes)
- **Detecting incomplete messages**. If we know the message length, we can determine whether a message is incomplete by comparing the number of received bytes with the expected message length. Without knowing the actual message length, we cannot be sure whether we have received the complete message. This is especially critical once we start downloading / uploading files, because the number of bytes received must match the expected file size.
- **Encode messages of different sizes**. We are not bound by some predefined message length. Each message can have its own size.

How do we encode the message length in the message we are about to send?

Let's take this message:

    ┌───┬───┬───┬───┬───┐
    │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┘
           5 bytes

This message is 5 bytes long. What if we could prepend a predefined number of bytes to this message to store a number representing the message length, like this:

    ┌───┬───┬───┬───┬───┬───┐
    │ 5 │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┴───┘
    └───┘ └─────────────────┘
    1 byte    5 bytes
    (message  (message data)
    length)   

           6 bytes (total)


That is one of the most common and simple ways to encode a message length.

One thing to pay attention to is that each block in the message is one byte in size. So by this logic, the number 5 is also represented in the form of 1 byte. If you remember how binary numbers work, you should know that the number 5 is represented as follows:

    00000101

Remember that the maximum value that a one-byte unsigned integer (`uint8_t`) can represent is 255. This means that a message longer than 255 bytes cannot be encoded if we use a single byte for message length.

To solve this, we can say that 4 bytes will be used for encoding a message length. This way, a message could look like this:

    ┌───┬───┬───┬───┬───┬───┬───┬───┬───┐
    │ 0 │ 0 │ 0 │ 5 │ H │ E │ L │ L │ O │
    └───┴───┴───┴───┴───┴───┴───┴───┴───┘
                    9 bytes

Since we use 4 bytes, we can encode messages up to 4,294,967,295 bytes in length.

The only predefined rule we need to follow is that the sender and the receiver have to know beforehand the number of bytes that will comprise a message length. For example, if a client sends a message in which two bytes encode the message length, the server must also know that the first two bytes represent the message length.

Now we have a basic understanding of how to encode a number representing a message length at the beginning of the message itself.


# 2. Implementation

In this section, our goal is to understand how to encode a message length on the client side and then how to process it on the server side.


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

What does this function do? The answer is quite simple. The function takes a `const std::string& message` parameter and returns a `std::vector<uint8_t>` containing the encoded message. 

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

Do not forget that each entry represents a binary value that occupies exactly one byte, so, more precisely, the message looks like this:

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

Notice how we encode the message length using hexadecimal numbers. You can read more about them [here](#5-hexadecimal-numbers).

We use hexadecimal because:

- It makes binary values easier to read.
- I just wanted to introduce hexadecimal numbers.

I think most of this function is fairly easy to understand, except for the following lines:

```C++
    uint16_t message_size = static_cast<uint16_t>(message_length);

    message_bytes.push_back(static_cast<uint8_t>(message_size >> 8));
    message_bytes.push_back(static_cast<uint8_t>(message_size));
```

Let's examine these lines in more detail. As shown in the previous diagram, we need to transform the original message `[I][][c][o][n][t][a][i][n][][-][][2][0][][b][y][t][e][s]` so that its first two bytes contain the encoded message length, represented in hexadecimal. The resulting encoded message should look like this: `[0x00][0x14][I][][c][o][n][t][a][i][n][][-][][2][0][][b][y][t][e][s]`.

First, we create a variable that occupies two bytes in C++: `uint16_t`, and cast the previously calculated `message_length` to that type. We need it because this is the size that we are going to use for message length encoding.

You may notice that the `std::vector<uint8_t> encode_message(const std::string& message)` function returns a vector of `uint8_t`, so where does the `uint16_t message_size` variable go? 

This is exactly where the bottom 2 lines come into play. These two lines use [bit shifting](https://www.geeksforgeeks.org/cpp/left-shift-right-shift-operators-c-cpp/) to transform the `uint16_t message_size` into two `uint8_t` numbers and append them to the encoded message vector. Here's how these two lines work:

    16-bit integer (uint16_t message_size):
    00000000 00010100

    Into two separate bytes:
    00000000 - 1st byte
    00010100 - 2nd byte

Great! Now we understand how to encode a message. Next, let's examine how the server (or any other receiver) processes an incoming message.


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

The goal of this function is simple. This function takes a socket containing buffered data of an unknown length and returns the message length, which cannot exceed the maximum value representable by two bytes (`uint16_t`).

Let's examine the function line by line.

```C++
uint16_t message_size_network = 0;
```

This is the variable that is going to store the actual message length.

```C++
size_t total_size_received = 0;
```

This variable stores the total number of bytes received. Remember that a single call to `recv(...)` is not guaranteed to return everything we need, so the server (or the receiver) needs to track how many bytes it has received in total.

```C++
while (total_size_received < BYTES_MESSAGE_LENGTH)
{
    ...
}
```

This is the main `while` loop, which continues executing as long as the number of received bytes is less than `BYTES_MESSAGE_LENGTH`. We need this loop because the server must ensure that it receives exactly `BYTES_MESSAGE_LENGTH` bytes. These bytes tell us the length of the incoming message.

```C++
ssize_t received = recv(socket_fd_client,
                        reinterpret_cast<uint8_t*>(&message_size_network) + total_size_received,
                        BYTES_MESSAGE_LENGTH - total_size_received,
                        0);

...

total_size_received += static_cast<size_t>(received);
```

The server reads the data from the socket and stores the number of received bytes in the `received` variable. Do not forget that `recv(...)` returns the number of received bytes.

The two important arguments we pass are:

- `reinterpret_cast<uint8_t*>(&message_size_network) + total_size_received`: a pointer to the first byte of the `message_size_network` + whatever `total_size_received` has. So if `total_size_received` is 0, then `reinterpret_cast<uint8_t*>(&message_size_network)` points to the first byte of `message_size_network`, if `total_size_received` is 1, then `reinterpret_cast<uint8_t*>(&message_size_network)` points to the second byte of `message_size_network`.
- `BYTES_MESSAGE_LENGTH - total_size_received`: read up to that many bytes from the socket.

```C++
total_size_received += static_cast<size_t>(received);
```

If `recv(...)` returns only a single byte, then the loop's termination condition has not yet been met, so we are going to have another iteration. Therefore, we need to update `total_size_received`, which tracks how many bytes we have already received.

The following diagram summarizes how the `while` loop reads the message length:

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

Last but not least, we need to understand how we handle problems while reading from the socket. We need these 3 `if` statements, because `recv(...)` does not always return the number of bytes received successfully. `recv(...)` can return the following values:

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

These `if` statements perform the following checks, in order:

- The first `if` statement checks whether `recv(...)` was interrupted by a signal while waiting for data. This is why we check `errno == EINTR`. Imagine that the server has started this loop and receives an interrupt signal while waiting for data (do not forget that if there are no bytes to be read in the socket, `recv(...)` becomes a blocking operation).

- The second `if` statement checks whether the client has closed its sending side of the connection. When this happens, `recv(...)` returns `0` once all pending data has been read.

- The third `if` statement handles all other errors when receiving bytes from the socket.

Great, now we know how to encode message lengths and read them from a socket.


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

As you may have noticed, these two loops, which are responsible for sending and receiving data, are very similar to the loop used by the server to read the message length. In fact, this is a common approach to designing programs that communicate through sockets. 

In a sense, the design follows these steps:

1. Determine how many bytes we need to transfer (send or receive)
2. Initialize a counter to track the transfer progress
3. Use a `while` loop to continue transferring bytes
4. Call `send(...)` or `recv(...)` (depending on the role) to transfer the remaining bytes
5. Handle possible errors
6. Update the counter after a successful operation
7. Repeat until all bytes have been transferred

The important thing to understand is that TCP is a byte-stream protocol, not a message-based protocol. TCP guarantees that bytes are delivered in order, but it does not guarantee that a single `send(...)` corresponds to a single `recv(...)`.

The following diagram illustrates a typical byte-transfer process:

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

In this lesson, we learned how to implement a simple length-prefixed protocol over TCP.

We covered how to encode a message using a predefined number of bytes and how to read the message correctly based on its decoded length. 

These simple concepts are fundamental building blocks for designing network systems. Many networking protocols use some form of message-length encoding, although it may be implemented in different ways. Encoding and reading message lengths allows the receiver to determine where each message begins and ends.


# 4. Hexadecimal Numbers

In C++, hexadecimal integer literals are written with the prefix `0x`. Here are a few examples:

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

- `hex` — Displays numbers in hexadecimal.
- `dec` — Displays numbers in decimal.
- `0x` — Prefix for hexadecimal literals in C++.
- Hexadecimal uses digits `0-9` and letters `A-F` (base 16).