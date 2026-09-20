# Network Learn



# Setup & Building

1. Clone the repository and initialize submodules:
    ```sh
    git clone --recursive https://github.com/ArturasDruteika/network_learn.git
    # or, if already cloned:
    git submodule update --init --recursive
    ```

2. Install build dependencies (Linux example):
    ```sh
    ./scripts/linux_setup.sh
    ```

3. Build the project:
    ```sh
    ./scripts/linux_build.sh
    ```

    Optionally, install the built project:
    ```sh
    ./scripts/linux_install.sh
    ```

4. Run the app (example):
    ```sh
    # EXAMPLE

    # On terminal 1:
    cd build/release/bin
    ./server_lesson_1

    # On terminal 2:
    cd build/release/bin
    ./client_lesson_1
    ```
# 🌐 Networking Learning Plan

> **Goal:** Learn computer networking from basic sockets to real-time multiplayer networking, then build a reusable networking system for my C++ 3D engine.

## Progress

- [x] Lesson 1 — TCP Ping Client/Server
- [ ] Phase 1 — Socket Fundamentals
- [ ] Phase 2 — Network Fundamentals
- [ ] Phase 3 — Internet Protocols
- [ ] Phase 4 — SSH
- [ ] Phase 5 — Tunneling & VPNs
- [ ] Phase 6 — 3D Engine Networking
- [ ] Phase 7 — Multiplayer Architecture
- [ ] Phase 8 — Optimization
- [ ] Phase 9 — Advanced Networking

---

# Phase 1 — Socket Fundamentals

## Lesson 1 — TCP Ping Client/Server ✅

**Goal:** Establish my first TCP connection.

Learn:

- TCP sockets
- Client/server architecture
- IP addresses
- Ports
- `socket()`
- `bind()`
- `listen()`
- `accept()`
- `connect()`
- `send()`
- `recv()`
- `close()`

Build:

```text
Client                         Server
  |                               |
  | -------- "PING" ------------> |
  | <------- "PONG" ------------- |
```

---

## Lesson 2 — TCP Message Length

**Goal:** Understand that TCP is a byte stream, not a message protocol.

Learn:

- Partial `send()`
- Partial `recv()`
- TCP buffering
- Message boundaries
- Message framing
- Disconnect handling

Build a length-prefixed protocol:

```text
+------------+----------------+
| Size       | Payload        |
| uint32_t   | N bytes        |
+------------+----------------+
```

---

## Lesson 3 — Multiple TCP Clients

**Goal:** Allow multiple machines to connect simultaneously.

Build:

```text
Client A ----+
             |
Client B ----+---- Server
             |
Client C ----+
```

Project:

**TCP chat server**

Learn:

- Multiple connections
- Per-client state
- Broadcasting messages
- Connection lifecycle

---

## Lesson 4 — Non-Blocking Networking

Learn:

- Blocking sockets
- Non-blocking sockets
- `select()`
- `poll()`
- Event-driven networking

Later investigate:

- Linux `epoll`
- Windows IOCP
- BSD/macOS `kqueue`

---

## Lesson 5 — UDP

Rebuild the ping project using UDP.

Learn:

- `SOCK_DGRAM`
- Datagrams
- `sendto()`
- `recvfrom()`
- Connectionless communication
- Why UDP differs from TCP

---

## Lesson 6 — Unreliable Networks

Build packet metadata:

```cpp
struct PacketHeader
{
    uint32_t sequence;
    uint32_t type;
};
```

Experiment with:

- Packet loss
- Duplicate packets
- Out-of-order packets
- Sequence numbers

---

## Lesson 7 — Binary Protocols & Serialization

Stop sending strings for everything.

Learn:

- Packet headers
- Binary serialization
- Deserialization
- Endianness
- Fixed-width integers
- Protocol IDs
- Protocol versions

Example:

```text
+----------+----------+----------+-------------+
| Protocol | Type     | Size     | Payload     |
+----------+----------+----------+-------------+
```

Build reusable:

```text
PacketWriter
PacketReader
```

classes for the engine.

---

# Phase 2 — How Networks Actually Work

## Lesson 8 — IPv4

Learn:

- IPv4 addresses
- Loopback
- `127.0.0.1`
- LAN addresses
- Public addresses
- Private address ranges
- CIDR
- Subnet masks
- Default gateways

Understand the difference between:

```text
localhost
LAN
Internet
```

---

## Lesson 9 — Ethernet & ARP

Learn:

- Ethernet frames
- MAC addresses
- Network interfaces
- Switches
- ARP
- Broadcasts

Understand:

```text
IP address
     ↓
ARP
     ↓
MAC address
     ↓
Ethernet frame
```

---

## Lesson 10 — Routing

Learn:

- Routing tables
- Default routes
- Gateways
- Routers
- Hops
- TTL

Experiment with:

```bash
traceroute
ip route
route
```

---

## Lesson 11 — ICMP

Learn:

- ICMP
- Echo request
- Echo reply
- TTL exceeded

Understand what the system `ping` command actually does.

---

## Lesson 12 — DNS

Learn:

```text
example.com
     ↓
DNS
     ↓
IP address
```

Explore:

- DNS resolvers
- DNS servers
- A records
- AAAA records
- CNAME
- DNS caching

Use:

```bash
nslookup
dig
```

Write a small C++ hostname resolver.

---

## Lesson 13 — NAT

Learn:

- Private IP addresses
- Public IP addresses
- NAT
- Port mappings
- Port forwarding
- CGNAT

Understand:

```text
PC
192.168.1.20:5000
       |
       v
    Router
       |
       v
PublicIP:42001
       |
       v
   Internet
```

---

## Lesson 14 — Firewalls

Learn:

- Incoming rules
- Outgoing rules
- Ports
- Stateful firewalls
- Why LAN connections work while Internet connections may fail

---

## Lesson 15 — Wireshark

Capture traffic from all previous projects.

Inspect:

- TCP handshake
- TCP sequence numbers
- ACKs
- UDP datagrams
- DNS
- ICMP
- HTTP

Frequently ask:

> What actually appeared on the wire?

---

# Phase 3 — Internet Protocols

## Lesson 16 — HTTP Client

Build a minimal HTTP/1.1 client using my TCP socket implementation.

Understand:

```text
HTTP
 ↓
TCP
 ↓
IP
```

Learn:

- Requests
- Responses
- Headers
- Status codes
- Content length
- Persistent connections

---

## Lesson 17 — HTTP Server

Build a tiny HTTP server.

Example:

```text
Browser
   |
   | HTTP
   v
My C++ Server
   |
   v
"Hello from my engine"
```

---

## Lesson 18 — WebSockets

Learn:

- HTTP Upgrade
- WebSocket handshake
- Frames
- Binary frames
- Ping/pong
- Persistent connections

Understand where WebSockets could fit:

```text
Engine ── WebSocket ── Backend

Browser ── WebSocket ── Backend
```

Possible future uses:

- Engine editor
- Debug dashboard
- Matchmaking
- Browser clients
- Live telemetry

---

## Lesson 19 — TLS

Learn conceptually:

- Encryption
- Hashing
- Public/private keys
- Symmetric encryption
- Authentication
- Certificates
- Certificate authorities
- TLS handshake

Do **not** create production cryptography myself.

---

## Lesson 20 — HTTPS

Use an established TLS library.

Understand:

```text
HTTP
  ↓
TLS
  ↓
TCP
  ↓
IP
```

---

# Phase 4 — SSH

## Lesson 21 — SSH Basics

Get access to another Linux machine or VM.

Learn:

```bash
ssh user@server
```

Understand:

```text
My PC
  |
  | TCP :22
  v
SSH Server
  |
  v
Remote Shell
```

Use SSH to run my C++ server remotely.

---

## Lesson 22 — SSH Keys

Learn:

```bash
ssh-keygen
```

Understand:

- Private keys
- Public keys
- `authorized_keys`
- Server host keys
- `known_hosts`
- Password authentication
- Public-key authentication

---

## Lesson 23 — SSH Protocol Internals

Study:

```text
TCP connection
      ↓
Version exchange
      ↓
Key exchange
      ↓
Server authentication
      ↓
Encryption established
      ↓
User authentication
      ↓
SSH channels
```

No need to implement SSH from scratch.

---

## Lesson 24 — SCP & SFTP

Deploy my game server.

```bash
scp GameServer user@server:/home/user/
ssh user@server
./GameServer
```

Learn basic remote server administration.

---

## Lesson 25 — SSH Port Forwarding

Learn:

```bash
ssh -L ...
ssh -R ...
ssh -D ...
```

Understand:

- Local forwarding
- Remote forwarding
- Dynamic forwarding
- SOCKS proxies

Draw the traffic path for every experiment.

---

## Lesson 26 — SSH Tunnels

Tunnel one of my C++ servers through SSH.

Compare traffic using Wireshark:

```text
WITHOUT SSH

Client ──────────────── Server


WITH SSH

Client
  |
  v
SSH Client
  |
  | encrypted tunnel
  v
SSH Server
  |
  v
Server
```

---

# Phase 5 — Tunneling & VPNs

## Lesson 27 — Encapsulation

Understand the idea of putting one protocol inside another.

Example:

```text
+---------------------------+
| Outer IP                  |
| UDP                       |
| +-----------------------+ |
| | Original IP packet    | |
| | TCP/UDP               | |
| | Application Data      | |
| +-----------------------+ |
+---------------------------+
```

---

## Lesson 28 — Proxies

Learn:

- Forward proxies
- Reverse proxies
- SOCKS
- HTTP proxies

Understand the difference between:

```text
Proxy
Tunnel
VPN
```

---

## Lesson 29 — Virtual Network Interfaces

Learn:

- TUN
- TAP
- Virtual adapters

Understand how applications can receive packets from the operating system.

---

## Lesson 30 — OS Routing

Go deeper into:

- Routing tables
- Interfaces
- Routes
- Gateways
- Metrics

Experiment with changing routes in a safe test environment.

---

## Lesson 31 — UDP Tunnel

Build:

```text
Machine A                         Machine B

Application                      Application
    |                                ^
    v                                |
Tunnel A ===== UDP socket =====> Tunnel B
```

Send arbitrary data through the tunnel.

---

## Lesson 32 — Toy VPN ⭐

Build a simple educational VPN.

```text
Machine A                              Machine B

Application                            Application
    |                                      ^
    v                                      |
 TCP/UDP                                TCP/UDP
    |                                      ^
    v                                      |
   IP                                     IP
    |                                      ^
    v                                      |
  TUN                                    TUN
    |                                      ^
    v                                      |
VPN Client ======= UDP Tunnel ======> VPN Server
```

Learn:

- Packet encapsulation
- Virtual interfaces
- Routing
- UDP transport
- Tunnel addressing

Security is **not** the goal of the toy implementation.

---

## Lesson 33 — Real VPN Architecture

Study established VPN protocols.

Compare concepts such as:

```text
WireGuard
OpenVPN
```

Study:

- Encryption
- Authentication
- Key exchange
- Peer configuration
- Routing
- Keepalives
- Replay protection

---

## Lesson 34 — NAT Traversal

Learn:

- NAT mappings
- UDP hole punching
- STUN
- TURN
- ICE
- Relay servers

Understand why:

```text
Player A behind NAT

        ???

Player B behind NAT
```

is harder than it initially appears.

---

# Phase 6 — Bring Networking Into the 3D Engine

## Lesson 35 — Networked Cube ⭐

Run two copies of my engine.

```text
ENGINE A             SERVER             ENGINE B

 Cube
   |
   | Transform
   +------------------>
                       |
                       +------------------> Cube
```

Replicate:

```cpp
position
rotation
```

Don't smooth anything yet.

---

## Lesson 36 — Authoritative Server

Move world authority to the server.

```text
CLIENT
   |
   | Input
   v
SERVER
   |
   | Simulation
   |
   | Authoritative State
   v
CLIENT
```

Learn the difference between:

```text
"I moved to X"

vs.

"I pressed forward"
```

---

## Lesson 37 — Network Ticks

Separate:

```text
Render Tick
Simulation Tick
Network Tick
```

Understand why they don't necessarily run at the same frequency.

---

## Lesson 38 — Snapshots

Server periodically sends:

```cpp
struct WorldSnapshot
{
    uint32_t tick;
    // entity states...
};
```

Maintain snapshot history.

---

## Lesson 39 — Snapshot Buffer

Store several snapshots instead of immediately displaying the newest one.

Learn about:

- Jitter
- Delayed packets
- Snapshot timing

---

## Lesson 40 — Interpolation

Smooth remote entities between known states.

```text
Snapshot A -------- Snapshot B
               ^
             render
```

---

## Lesson 41 — Extrapolation

Predict briefly when snapshots don't arrive.

Then correct when authoritative data returns.

---

## Lesson 42 — Client-Side Prediction

Don't wait for the server before moving the local player.

```text
INPUT
  |
  +----> Local simulation ----> Render
  |
  +----> Server
```

---

## Lesson 43 — Server Reconciliation

Keep input history:

```text
Input 100
Input 101
Input 102
Input 103
```

Receive authoritative state and replay unacknowledged inputs.

---

# Phase 7 — Engine Networking Architecture

Create a proper engine subsystem:

```text
Engine
│
├── Renderer
├── Physics
├── Audio
├── Input
├── Scene
│
└── Network
    ├── Socket
    ├── Address
    ├── Connection
    ├── Packet
    ├── PacketReader
    ├── PacketWriter
    ├── NetworkObject
    ├── Replication
    └── RPC
```

## Lesson 44 — Network Entity IDs

Every replicated object receives an identifier.

---

## Lesson 45 — Spawn / Despawn

Replicate entity lifecycle.

```text
SERVER

Spawn Entity #42
       |
       v
CLIENT

Create Entity #42
```

---

## Lesson 46 — Property Replication

Replicate things such as:

```text
Transform
Velocity
Animation state
Health
Physics state
```

---

## Lesson 47 — RPCs / Network Events

Implement events such as:

```text
FireWeapon()
PlayAnimation()
OpenDoor()
Respawn()
```

Understand when to replicate **state** versus send an **event**.

---

## Lesson 48 — Ownership

Learn:

- Server authority
- Client ownership
- Network authority
- Ownership transfer

---

## Lesson 49 — Reliable/Unreliable Channels

Some messages need reliability:

```text
PlayerSpawned
InventoryChanged
```

Some generally don't:

```text
Transform snapshot #100
Transform snapshot #101
Transform snapshot #102
```

Build reliability over UDP as an educational exercise.

---

## Lesson 50 — Connection Management

Handle:

- Connect
- Disconnect
- Timeout
- Reconnect
- Heartbeats
- Connection state

---

## Lesson 51 — Protocol Versioning

Make the protocol capable of evolving without immediately becoming a nightmare.

---

# Phase 8 — Optimization

## Lesson 52 — Measure Bandwidth

Build statistics:

```text
Packets/sec
Bytes/sec
Bytes/entity
Bytes/snapshot
Packet loss
RTT
Jitter
```

---

## Lesson 53 — Bit Packing

Stop wasting entire integers where only a few bits are required.

---

## Lesson 54 — Quantization

Instead of:

```text
float X
float Y
float Z

96 bits
```

experiment with quantized coordinates.

Measure:

```text
Bandwidth saved
vs.
Precision lost
```

---

## Lesson 55 — Delta Compression

Instead of sending:

```text
Entire World
Entire World
Entire World
Entire World
```

send changes relative to previously known state.

---

## Lesson 56 — Packet Budgets

Decide how much bandwidth each network tick is allowed to consume.

Prioritize important updates.

---

## Lesson 57 — Interest Management

Don't send every entity to every client.

```text
             WORLD

       A       B       C

    Player 1       Player 2
       |               |
   cares about      cares about
      A/B              B/C
```

---

## Lesson 58 — Spatial Networking

Reuse engine spatial structures where appropriate:

```text
BVH
Octree
Grid
Zones
```

Rendering asks:

> What objects does this camera need?

Networking asks:

> What objects does this client need?

---

# Phase 9 — Advanced Multiplayer

## Lesson 59 — Lag Compensation

Learn historical world states and server rewind.

---

## Lesson 60 — Rollback

Learn:

- Input history
- State history
- Rollback
- Re-simulation

---

## Lesson 61 — Deterministic Simulation

Investigate:

- Floating-point determinism
- Fixed timestep simulation
- Lockstep networking

Compare different multiplayer architectures.

---

## Lesson 62 — Networking Security

Learn to distrust network input.

Investigate:

- Malformed packets
- Invalid packet lengths
- Fake entity IDs
- Invalid inputs
- Rate limiting
- Replay attacks
- Authentication
- DoS considerations

---

## Lesson 63 — Dedicated Servers

Separate:

```text
Game Client
```

from:

```text
Dedicated Game Server
```

Run the dedicated server remotely through everything learned earlier about Linux and SSH.

---

## Lesson 64 — Matchmaking & Lobbies

Study the difference between:

```text
Backend services
       |
       +---- Authentication
       +---- Matchmaking
       +---- Lobbies
       +---- Server discovery

Game Server
       |
       +---- Real-time simulation
```

---

## Lesson 65 — Production Debugging

Build networking diagnostics directly into the engine.

Display:

```text
Ping:            43 ms
Jitter:           7 ms
Packet Loss:    1.2 %
Download:       34 KB/s
Upload:         12 KB/s
Server Tick:       60
Snapshot Rate:     20
```

Add artificial:

```text
Latency
Jitter
Loss
Duplication
Reordering
```

Never rely solely on perfect localhost conditions.

---

# Final Project — Engine Networking System ⭐

The final goal is a reusable networking layer integrated with the 3D engine.

```text
                 GAME BACKEND
                /            \
          HTTP / WS         Database
              |
              |
        DEDICATED SERVER
              |
       UDP Game Protocol
        /             \
       /               \
 CLIENT A             CLIENT B
    |                     |
Renderer               Renderer
Physics                 Physics
Scene                   Scene
Network                 Network
```

The engine should eventually support:

- [ ] Client/server connections
- [ ] Dedicated servers
- [ ] Binary serialization
- [ ] UDP transport
- [ ] Reliable messages
- [ ] Unreliable messages
- [ ] Network entity IDs
- [ ] Spawn/despawn replication
- [ ] State replication
- [ ] RPC/events
- [ ] Ownership/authority
- [ ] Server ticks
- [ ] Snapshots
- [ ] Interpolation
- [ ] Client prediction
- [ ] Server reconciliation
- [ ] Interest management
- [ ] Bandwidth optimization
- [ ] Connection timeouts
- [ ] Network debugging statistics
- [ ] Artificial latency/loss testing
- [ ] Protocol versioning
- [ ] Input validation/security
- [ ] Remote dedicated-server deployment

---

# Rules for Learning

## 1. Build Everything Small First

Don't immediately put every experiment into the engine.

```text
tiny console experiment
        ↓
understand it
        ↓
make it reusable
        ↓
integrate into engine
```

## 2. Don't Just Use Protocols — Inspect Them

Use Wireshark regularly.

If I write:

```cpp
send(socket, data, size, 0);
```

I should eventually be able to explain roughly what happens between that call and the receiving application.

## 3. Break the Network Intentionally

Test with:

```text
0 ms latency
50 ms latency
150 ms latency
300 ms latency

0% packet loss
1% packet loss
5% packet loss
10% packet loss

jitter
duplication
reordering
```

Networking code that only works on localhost doesn't count as finished.

## 4. Don't Implement Production Cryptography

Implementing protocols for education is useful.

Inventing cryptography for real security is not.

Use established cryptographic/TLS/SSH/VPN implementations for anything real.

## 5. Keep Asking What's Underneath

Follow the stack downward:

```text
My Engine
    ↓
Network System
    ↓
Socket API
────────────────
    ↓
TCP / UDP
    ↓
IP
    ↓
Routing
    ↓
Ethernet / Wi-Fi
    ↓
NIC
    ↓
Physical Network
```

Then follow it upward:

```text
TCP / UDP
    ↓
HTTP / TLS / SSH / WebSocket
    ↓
Backend services
    ↓
Game protocols
    ↓
Replication
    ↓
Multiplayer world
```

---

# End Goal

The goal isn't merely to know how to call:

```cpp
send();
recv();
```

The goal is to understand:

> **How can a real-time 3D world exist consistently across multiple computers connected by an unreliable network?**

And then build that system myself.