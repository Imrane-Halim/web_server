# Event Loop Architecture - Implementation Guide

## Overview

This document describes the best practices implemented in the event-driven web server architecture using Linux epoll.

## Architecture Components

### 1. EventLoop (src/server/EventLoop.cpp)

**Responsibilities:**
# Event loop architecture (overview & file map)

This document explains the event loop design used in this repository and points to the files that implement each responsibility. The goal is to make it easy to find where behavior is implemented and which files to inspect or change.

Files of interest
- Event loop and main loop: `src/server/EventLoop.cpp` and header `include/server/EventLoop.hpp`
- Epoll wrapper and helpers: `include/server/Epoll.hpp`, `src/server/Epoll.cpp`
- FD management: `include/server/FdManager.hpp`, `src/server/FdManager.cpp`
- Server socket and accept logic: `include/server/Server.hpp`, `src/server/Server.cpp`
- Client connection lifecycle: `include/server/Client.hpp`, `src/server/Client.cpp`

Core responsibilities
- EventLoop: polls epoll, dispatches ready events to handlers, and coordinates shutdown (uses `g_shutdown`). See `src/server/EventLoop.cpp` for the main loop, timeout choice, and top-level exception handling.
- FdManager: single place that maps FDs to handler objects, registers/modifies/removes FDs from epoll and provides existence checks to avoid use-after-free.
- Client: per-connection handler that performs reads, parsing, state transitions and writes. Client objects may self-delete after cleanup — `FdManager` checks help avoid races.

Design points and best practices (implemented here)
- Level-triggered epoll (LT): chosen for simplicity and reliability. The implementation registers sockets with `EPOLLIN`/`EPOLLOUT` (no `EPOLLET`). See `docs/LEVEL_TRIGGERED_IMPLEMENTATION.md` for rationale.
- Error-first processing: when an event reports an error flag, handlers process error conditions before regular read/write handling.
- Timeout-based epoll_wait: EventLoop uses a short timeout (1s) so signal-based shutdown (SIGINT/SIGTERM) is handled promptly.
- Handler existence validation: after processing one event, code checks whether the handler still exists in `FdManager` before handling subsequent events for the same FD in the same loop iteration.
- Self-deletion safety: clients remove themselves from `FdManager` before deleting. EventLoop and FdManager provide support to avoid use-after-free.

Quick pointers to implementation patterns
- Signal handling and shutdown: search for `g_shutdown` and `signal` in `src/server/EventLoop.cpp`.
- Registering and modifying events: `FdManager::add`, `FdManager::modify`, `FdManager::remove` in `include/server/FdManager.hpp` and `src/server/FdManager.cpp`.
- Read/write state transitions: `src/server/Client.cpp` — see `onReadable`, `onWritable`, and internal state machine.

If you plan to change the event loop behavior, update `docs/EVENT_LOOP_ARCHITECTURE.md` and the specific implementation files above so future contributors can follow the rationale and code path.
