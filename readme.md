# mini_redis

A Redis-compatible in-memory key-value store built from scratch in C++,
implementing the core KV engine, RESP wire protocol, concurrency, and
persistence directly over raw POSIX sockets — no external networking or
Redis libraries used.

## Why

Real Redis is written in C and achieves its performance through manual
memory management and direct control over I/O. This project was built
to understand that by working at the same abstraction level: raw
sockets, a hand-rolled RESP parser, thread-per-connection concurrency,
and an append-only log for durability.

## Features

- **TCP server** built directly on raw POSIX sockets (`socket`, `bind`,
  `listen`, `accept`)
- **RESP protocol** support — compatible with the real `redis-cli`, not
  just a custom client
- **Core commands**: `SET`, `GET`, `DEL`, `PING`
- **AOF (Append-Only File) persistence** — every write is logged to
  disk and replayed on startup, so data survives a hard process kill
- **Concurrent clients** via thread-per-connection, with a mutex-protected
  shared store and mutex-protected AOF writes

## Benchmarks

Measured with a Python load test script (20 concurrent clients, 500
SET+GET pairs each, 20,000 total operations):

| Metric | Value |
|---|---|
| Throughput | ~2,300 ops/sec |
| p50 latency | ~15 ms |
| p99 latency | ~60 ms |

Run it yourself:
```bash
python3 -m venv venv
source venv/bin/activate
pip install redis
python3 tests/load_test.py
```

## Build

```bash
mkdir build && cd build
cmake ..
make
./mini_redis
```

Server listens on port `6380` by default.

## Try it

```bash
redis-cli -p 6380
127.0.0.1:6380> SET name abc
OK
127.0.0.1:6380> GET name
"abc"
127.0.0.1:6380> DEL name
(integer) 1
```

## Design notes

- **Concurrency model**: thread-per-connection rather than an event
  loop (epoll). Simpler to reason about at this scale; a natural next
  step would be moving to an event loop for higher connection counts
  without per-thread overhead.
- **Persistence**: append-only log, not snapshotting. Every `SET`/`DEL`
  is RESP-encoded and appended to `appendonly.aof` before responding
  to the client. On restart, the log is replayed in order to rebuild
  state.
- **Consistency**: a single mutex guards the entire in-memory store.
  This is correct but means writes serialize under high concurrency —
  a natural extension would be sharding the keyspace across multiple
  locks to reduce contention.

## Tech

C++17, CMake, raw POSIX sockets. Built and tested on WSL2 (Ubuntu).