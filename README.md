````markdown
# KTP Socket Protocol (Networks Project)

**Author:** Konduri Jeevan Varma  
**Roll No:** 22CS10038  

---

## 📖 Overview
This project implements a **custom socket protocol (KTP)** on top of UDP sockets.  
It uses **shared memory**, **semaphores**, and **mutexes** for synchronization, and applies a **sliding window mechanism** to achieve reliable communication over UDP.

The implementation supports:
- Socket creation
- Binding
- Sending & receiving
- Closing sockets
- Reliability through retransmission and ACK-based sliding windows

---

## ⚙️ Features
- Custom socket type: **SOCK_KTP**
- Sliding window protocol for reliability
- Simulated packet loss with configurable probability
- Thread-based design:
  - Sender thread
  - Receiver thread
  - Garbage collector thread
- Shared memory and semaphores for inter-process communication
- Error handling via `SOCK_INFO`

---

## 🛠️ Macros and Constants
- **SOCK_KTP** → Identifier for KTP socket type  
- **T** → Timeout period for retransmission  
- **MAX_SOCKETS** → Maximum sockets supported (25)  
- **DROP_PROBABILITY** → Probability of simulating packet drop  
- **SEND_BUFFER_SIZE / RECV_BUFFER_SIZE** → Buffer sizes  
- **SENDER_WINDOW_SIZE / RECEIVER_WINDOW_SIZE** → Sliding window sizes  
- **MAX_SEQ_NUM** → Maximum sequence number (wraps at 256)  
- **MESSAGE_SIZE** → 512 bytes per message  
- **IP_SIZE** → 16 (for storing IP addresses)  
- **ERROR / SUCCESS** → Standard return codes  

---

## 📂 Data Structures
- **int8u** → 8-bit unsigned integer with modulo arithmetic  
- **header** → Holds sequence number for messages  
- **message** → Contains header + payload  
- **send_window** → Tracks outgoing messages, sequence numbers, ACKs  
- **receive_window** → Tracks incoming messages, reordering, consumption  
- **ktpSocket** → Represents a socket instance with buffers, windows, IP/port  
- **SOCK_INFO** → Used to share metadata (status, errors, return values)  

---

## 🔑 Socket API Functions
- `int k_socket(int domain, int type, int protocol)` → Create KTP socket  
- `int k_bind(int socket_id, char *src_ip, unsigned short src_port, char *dest_ip, unsigned short dest_port)` → Bind socket  
- `int k_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)` → Send data  
- `int k_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)` → Receive data  
- `int k_close(int socket_id)` → Close socket  

---

## 🧵 Threads
- **Sender Thread** → Retransmits unacknowledged packets  
- **Receiver Thread** → Receives packets, updates window, sends ACKs  
- **Garbage Collector Thread** → Cleans up terminated sockets  

---

## 🔒 Synchronization
- **Semaphores (`sem1`, `sem2`)** → Inter-thread synchronization  
- **Mutexes** → Protect shared resources:
  - Socket table
  - Send/receive windows
  - Buffers  

---

## 📊 Simulation Results
For a **77KB file (~153 messages, each 512B)**, the effect of different packet loss probabilities (`p`) is shown below:

| Probability | Total Messages | Expected Transmissions | Avg Transmissions per Message |
|-------------|----------------|------------------------|-------------------------------|
| 0.05        | 153            | 167                    | 1.09                          |
| 0.10        | 153            | 173                    | 1.13                          |
| 0.15        | 153            | 174                    | 1.14                          |
| 0.20        | 153            | 205                    | 1.34                          |
| 0.25        | 153            | 219                    | 1.43                          |
| 0.30        | 153            | 208                    | 1.36                          |
| 0.35        | 153            | 220                    | 1.44                          |
| 0.40        | 153            | 237                    | 1.55                          |
| 0.45        | 153            | 246                    | 1.61                          |
| 0.50        | 153            | 289                    | 1.89                          |

---

## 🚀 How to Run
1. Compile the project:
   ```bash
   make
````

2. Start the initializer (sets up shared memory & semaphores):

   ```bash
   ./initksocket
   ```
3. Run user programs:

   ```bash
   ./user1
   ./user2
   ```
4. Clean up resources on exit (SIGINT handler takes care of removal).

---

## 📌 References

* \[1] Tanenbaum, Computer Networks
* \[2] Peterson & Davie, Computer Networks: A Systems Approach
* \[3] Kurose & Ross, Computer Networking: A Top-Down Approach

---

## 📜 License

This project is for educational purposes only under the Networks Project coursework.

```


