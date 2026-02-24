# SecureNetChat

A C++ TCP-based chat application with customizable network topologies and built-in cryptography. Developed as part of the **Computernetze Praktikum** at **TU Braunschweig**.

## Features

- TCP-based chat for multiple users
- Design and configuration of custom network topologies
- Secure communication using cryptographic mechanisms

## Getting Started

1. Clone the repository:

    git clone https://github.com/yourusername/SecureNetChat.git
    cd SecureNetChat

2. Build the project (CMake):

    mkdir build && cd build
    cmake ..
    make

3. Run the application:

    ./SecureNetChat

## Requirements

- C++17 or higher
- CMake 3.15+
- OpenSSL (for cryptography)

---

## Commands

| Command    | Description                       |
|-----------|-----------------------------------|
| JOIN      | Join a group                       |
| LEAVE     | Leave the group                     |
| NICK      | Change own nickname                 |
| LIST      | List all existing groups            |
| GETMEMBERS| Lists all users of the group        |
| GETTOPIC  | Prints the current topic of the group |
| SETTOPIC  | Sets the current topic of the group |
| MSG       | Message a single user or group      |
| NEIGHBORS | Lists direct neighbors              |
| PING      | Determines availability and RTT to destination |
| ROUTE     | Shows route to destination including individual hops |
| PLOT      | Plots topology of network           |
| QUIT      | Leave IBRC                           |

---

## Type of Communication

P2PC also has two types of communication:

- **One-to-many:** This corresponds to the group-based form, i.e., messages are sent to all users in a group.  
- **One-to-one:** A communication between exactly two clients takes place.
