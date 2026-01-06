# World Cup 2026 Informer - STOMP Implementation

[cite_start]This project is a "community-led" world cup update subscription service implemented using the **STOMP** (Simple-Text-Oriented-Messaging-Protocol) standard[cite: 7, 12]. [cite_start]It consists of a Java-based server and a C++ client, allowing users to subscribe to game channels and receive real-time updates from other users[cite: 8, 9, 10, 11].

## Project Structure

* [cite_start]**Server (Java):** A STOMP server that supports both **Thread-Per-Client (TPC)** and **Reactor** architectures[cite: 10]. [cite_start]It manages subscriptions, distributes messages to topics, and handles user authentication[cite: 46, 48, 62].
* [cite_start]**Client (C++):** A multi-threaded client designed to handle simultaneous keyboard input and server socket communication[cite: 11, 290, 291].
* [cite_start]**Protocol:** Implements **STOMP 1.2**, supporting frames such as `CONNECT`, `SUBSCRIBE`, `SEND`, `UNSUBSCRIBE`, and `DISCONNECT`[cite: 22, 74].
* [cite_start]**Database:** Integrates with an **SQLite** database via a Python bridge for persistent tracking of users, logins, and report files[cite: 261, 262, 271].
