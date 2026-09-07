# mini-redis

A Redis-like in-memory key-value store built from scratch in C, using Linux TCP sockets and an `epoll`-based event loop to handle multiple concurrent client connections on a single thread.

The project implements the core components of a networked key-value store: custom in-memory storage, TCP communication, event-driven I/O, command parsing, and append-only persistence.

## Features

* `SET key value`, `GET key`, and `DEL key` commands over a TCP text protocol
* Custom hash table with separate chaining
* `djb2` hash function with collision handling through linked lists
* Single-threaded event loop using Linux `epoll`
* Multiple concurrent TCP client connections without a thread-per-client model
* Append-only file (AOF) persistence
* Automatic AOF replay on server startup
* Memory leak testing with Valgrind
* Performance benchmarks for single-client and concurrent-client workloads

## Architecture

```text
                         ┌─────────────────┐
                         │   TCP Client    │
                         └────────┬────────┘
                                  │
                                  ▼
                         ┌─────────────────┐
                         │   epoll loop    │
                         │  event handling │
                         └────────┬────────┘
                                  │
                                  ▼
                         ┌─────────────────┐
                         │ Command Parser  │
                         └────────┬────────┘
                                  │
                                  ▼
                         ┌─────────────────┐
                         │   Hash Table    │
                         │  In-memory KV   │
                         └────────┬────────┘
                                  │
                         write operations
                                  │
                                  ▼
                         ┌─────────────────┐
                         │     AOF Log     │
                         │      Disk       │
                         └─────────────────┘
```

## Project Structure

| Module      | File               | Responsibility                                                |
| ----------- | ------------------ | ------------------------------------------------------------- |
| Hash table  | `hashtable.c/.h`   | In-memory key-value storage using separate chaining           |
| TCP server  | `server.c/.h`      | Socket creation, binding, and listening                       |
| Event loop  | `main.c`           | `epoll` event loop, connection handling, and request dispatch |
| Parser      | `parser.c/.h`      | Parses raw TCP input into structured commands                 |
| Persistence | `persistence.c/.h` | AOF logging and replay                                        |
| Tests       | `test_*.c`         | Parser and server tests                                       |

## Building

Compile with GCC:

```bash
gcc -Wall -Wextra -g \
    -o mini_redis \
    main.c server.c parser.c hashtable.c persistence.c
```

Run the server:

```bash
./mini_redis
```

The server listens on TCP port `6379`.

## Usage

Connect using `netcat`:

```bash
nc localhost 6379
```

Then send commands:

```text
SET name Anne
OK

GET name
Anne

DEL name
OK

GET name
(nil)
```

## Persistence

mini-redis uses an **Append-Only File (AOF)** to persist write operations.

For example:

```text
SET name Anne
SET language C
DEL language
```

These commands are appended to the AOF log and replayed when the server starts again.

The persistence layer provides durability for write operations while introducing additional disk I/O overhead.

## Performance

Benchmarks were performed using a Python client sending `10,000 SET` commands.

| Scenario                               |    Throughput |
| -------------------------------------- | ------------: |
| No persistence, single client          | 7,773 ops/sec |
| AOF persistence, single client         |   963 ops/sec |
| AOF persistence, 10 concurrent clients | 1,348 ops/sec |

### Performance Investigation

The initial AOF implementation opened and closed the log file for every write. This reduced throughput to approximately **60 ops/sec**, compared with **7,773 ops/sec** without persistence.

The repeated `fopen`/`fclose` operations introduced significant overhead. The benchmark was also affected by running the project on a Windows-mounted filesystem under WSL.

The persistence implementation was changed to keep the file handle open for the lifetime of the server and flush after each write.

This increased throughput to approximately **963 ops/sec**, a roughly **16× improvement**, while maintaining the durability guarantee that each AOF entry is flushed before the corresponding response is sent to the client.

### Concurrent Clients

With 10 concurrent clients, throughput increased to approximately **1,348 ops/sec**, but did not scale linearly.

The AOF layer remains a shared disk-I/O bottleneck. The `epoll` event loop allows multiple network connections to be processed concurrently, but persistence still serializes writes through the same storage path.

This highlights the difference between network-level concurrency and storage-level throughput.

## Testing

The project includes tests for parser and server functionality.

Memory correctness was checked using Valgrind:

```bash
valgrind --leak-check=full ./mini_redis
```

The project is tested for memory leaks and invalid memory accesses across the main request paths.

## Systems Concepts

* C memory management
* Hash tables and collision handling
* Linked lists
* TCP sockets
* Linux file descriptors
* `epoll`
* Event-driven I/O
* Client/server architecture
* Protocol parsing
* File I/O
* Append-only persistence
* Recovery through log replay
* Performance benchmarking
* Bottleneck analysis
* Memory debugging with Valgrind

## Future Improvements

* Per-client input buffers for partial TCP messages
* Pipelined commands
* Non-blocking sockets throughout the event loop
* Batched AOF writes
* AOF compaction
* Periodic snapshots
* More robust handling of partial reads and writes
* Additional commands such as `EXISTS` and `INCR`
* Extended benchmarking across different persistence configurations