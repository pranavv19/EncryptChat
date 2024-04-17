# SecureChatC

SecureChatC is a multi-client chat server application in C, featuring message encryption, chat history management, and support for private/group messaging. It showcases network programming, threading, and security basics.

## Features

- Multi-client support with a centralized server
- Message encryption for secure communication
- Chat history management for each client
- Private and group messaging capabilities

## Getting Started

### Prerequisites

- GCC Compiler
- POSIX compliant system for threading

### Installation

Clone the repository and compile the code:

```bash
git clone https://github.com/yourusername/SecureChatC.git
cd SecureChatC
gcc server.c -o server -lpthread
gcc client.c -o client -lpthread
Running the Application
```
Start the server:
```bash
./server
```
Connect a client:
```bash
./client <Server IP> <Port>
```
