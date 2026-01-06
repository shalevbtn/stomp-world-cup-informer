# World Cup 2026 Informer - STOMP Implementation

A community-led World Cup update service allowing users to subscribe to game channels and share real-time reports. The system utilizes a Java-based server and a multi-threaded C++ client communicating via the STOMP 1.2 protocol.

## Project Architecture
* **Java Server:** Implements both **Thread-Per-Client (TPC)** and **Reactor** concurrency models.
* **C++ Client:** A multi-threaded application that handles simultaneous terminal input and network socket communication.
* **Protocol:** Full implementation of **STOMP 1.2** (Simple-Text-Oriented-Messaging-Protocol) for asynchronous message passing.
* **Database:** Integration with **SQLite** for persistent storage of user credentials, login history, and file tracking.

## Skills & Concepts Implemented
* **Network Programming:** TCP/IP socket communication and custom protocol implementation.
* **Concurrency & Multithreading:** Managing multiple client connections (Java) and simultaneous I/O threads (C++).
* **Software Design Patterns:** Utilization of the Reactor and Template patterns for scalable server design.
* **Database Management:** Relational data modeling and SQL query execution.
* **Systems Programming:** Cross-language development using Maven (Java) and Makefiles (C++).
