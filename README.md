# ft_irc

An IRC server written in C++98, compliant with the IRC protocol (RFC 2812).
The server handles multiple clients simultaneously using `poll()` for non-blocking I/O multiplexing.

## Features

- **Authentication** with server password (`PASS`)
- **User registration** (`NICK`, `USER`)
- **Channel management** (`JOIN`, `PART`, `KICK`, `INVITE`, `TOPIC`)
- **Private messaging** (`PRIVMSG` to channels and users)
- **Channel modes**:
  - `+i` / `-i` — Invite-only channel
  - `+t` / `-t` — Topic restricted to operators
  - `+k` / `-k` — Channel key (password)
  - `+l` / `-l` — User limit
  - `+o` / `-o` — Channel operator privilege
- **PING/PONG** keep-alive
- **Graceful signal handling** (`SIGINT`, `SIGQUIT`)
- Non-blocking sockets with `poll()` multiplexing

## Requirements

- C++ compiler with C++98 support
- Make
- Linux / macOS

## Build

```bash
make        # Build the project
make clean  # Remove object files
make fclean # Remove object files and binary
make re     # Rebuild from scratch
```

## Usage

```bash
./ircserv <port> <password>
```

| Argument   | Description                              |
|------------|------------------------------------------|
| `port`     | Port number to listen on (1024–65535)    |
| `password` | Connection password required by clients  |

### Example

```bash
./ircserv 6667 mypassword
```

## Connecting

Use any IRC client (e.g., **irssi**, **WeeChat**, **HexChat**) or `nc`:

```bash
nc localhost 6667
```

Then authenticate and register:

```
PASS mypassword
NICK mynick
USER myuser 0 * :My Real Name
```

## Supported Commands

| Command   | Description                                  |
|-----------|----------------------------------------------|
| `PASS`    | Authenticate with server password            |
| `NICK`    | Set or change nickname                       |
| `USER`    | Set username and realname                    |
| `JOIN`    | Join a channel (with optional key)           |
| `PART`    | Leave a channel                              |
| `PRIVMSG` | Send a message to a channel or user         |
| `KICK`    | Remove a user from a channel (operator only) |
| `INVITE`  | Invite a user to a channel                   |
| `TOPIC`   | View or set a channel topic                  |
| `MODE`    | Set channel modes (+i, +t, +k, +l, +o)      |
| `PING`    | Keep-alive request                           |
| `QUIT`    | Disconnect from the server                   |

## Project Structure

```
ft_irc/
├── Makefile
├── inc/
│   ├── Server.hpp
│   ├── Client.hpp
│   └── Chanell.hpp
├── src/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   └── Chanell.cpp
└── cmd/
    ├── join.cpp
    ├── part.cpp
    ├── kick.cpp
    ├── invite.cpp
    ├── topic.cpp
    ├── mode.cpp
    ├── privmsg.cpp
    └── utls.cpp
```

