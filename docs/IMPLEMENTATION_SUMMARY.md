# Event Loop Implementation - Changes Summary

## Overview

This document summarizes the improvements and best practices implemented to complete the event loop client and server implementation.

# Implementation summary (what to look for)

This document summarizes where major implementation choices appear in the codebase and lists the key files you should inspect when changing behavior.

Short file map of implemented behavior
- Top-level loop and shutdown: `src/server/EventLoop.cpp` — timeout in `epoll_wait`, signal handling and top-level dispatch.
- Server socket and accept: `src/server/Server.cpp` and `include/server/Server.hpp` — socket options (SO_REUSEADDR), non-blocking accept and client creation.
- Client lifecycle and HTTP I/O: `src/server/Client.cpp` and `include/server/Client.hpp` — read/write handlers, state machine, partial write tracking.
- FD registration & lookup: `include/server/FdManager.hpp` and `src/server/FdManager.cpp` — add/modify/remove helpers and existence checks.
- Epoll thin wrapper: `include/server/Epoll.hpp` and `src/server/Epoll.cpp`.
- HTTP parsing / request handling: `include/http/HTTPParser.hpp`, `include/http/RequestHandler.hpp` and `src/http/` implementations.
- CGI handling: `include/cgi/CGIHandler.hpp` and `src/cgi/CGIHandler.cpp`.

Key patterns implemented
- Level-triggered epoll (LT) is used to simplify read/write handling and reduce the risk of missed events.
- Error-first processing: handlers check and handle error flags before normal read/write flow.
- Handler existence checks: `FdManager::exists()` is used to avoid processing after a handler has been removed/deleted.
- Self-deletion pattern for clients: clients remove their FD from `FdManager` and then `delete this` (code must not use the object afterward).
- Timeout in epoll_wait (short timeout) to ensure signals are observed promptly.

Quick actions when modifying behaviour
1. Change event dispatch semantics: edit `src/server/EventLoop.cpp` and ensure `FdManager` checks remain.
2. Change how clients are registered: update `src/server/Server.cpp` and `src/server/Client.cpp` together.
3. Add new events or handler types: extend the EventHandler interface in `include/server/EventHandler.hpp` (if present) and update FdManager accordingly.

Testing and verification
- Build: `make` (see `Makefile`) — this repository provides a Makefile that compiles the sources under `src/`.
- Smoke tests: use files in `test/` and `www/` to exercise static serving, file uploads and CGI scripts.
- Runtime checks: verify graceful shutdown (SIGINT), partial write handling (large file transfers), and client disconnect scenarios.

If you want, I can open a PR that adds a short checklist to each of these files (TODO comments) so future contributors know what to update when changing behavior.
