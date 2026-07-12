# mini_redis

A minimal Redis clone built from scratch in C++, implementing the core
KV engine, RESP wire protocol, and persistence layer directly over raw
TCP sockets — no external networking or Redis libraries.

## Why

Real Redis is written in C and achieves its performance through manual
memory management and low-level control over I/O. This project is an
attempt to understand that by building at the same abstraction level:
raw sockets, a hand-rolled protocol parser, and an append-only log for
durability.

## Status

Early development — currently setting up the TCP server and build
system. Core KV logic, RESP parsing, and persistence are in progress.

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Tech

C++17, CMake, raw POSIX sockets (Linux/WSL2).