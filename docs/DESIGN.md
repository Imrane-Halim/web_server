note: this is just an initial thought about how the projects' structure nothing is certain!

this is how the project is PROBABBLY gonna be structured, this might change in the future:

```
webserv/
├── Makefile               # Build script
├── include/               # Header files
│   ├── server/            # Server core headers
│   │   ├── Server.hpp
│   │   ├── Connection.hpp
│   │   └── EventLoop.hpp
│   ├── config/            # Configuration parsing
│   │   ├── ConfigParser.hpp
│   │   ├── GlobalConfig.hpp
│   │   ├── ServerConfig.hpp
│   │   └── RouteConfig.hpp
│   ├── http/              # HTTP protocol handling
│   │   ├── Request.hpp
│   │   ├── Response.hpp
│   │   └── StatusCodes.hpp
│   ├── cgi/               # CGI execution
│   │   └── CGIHandler.hpp
│   └── utils/             # Utilities
│       ├── Logger.hpp
│       ├── FileUtils.hpp
│       ├── SharedPtr.hpp  # To make our life easier
````markdown
## Project design and structure

This document describes the intended project layout and where the main components live in this repository. The docs are intentionally short and point directly at the source and header files so you can quickly navigate the implementation.

Top-level layout (actual repo):

```
web_server/
├── Makefile                # Build script
├── include/                # Public headers (mirrors src/)
│   ├── server/             # Server core headers (EventLoop, Server, Client helpers)
│   ├── http/               # HTTP parsing and request/response types
│   ├── cgi/                # CGI handler header
│   └── utils/              # Logger, RingBuffer, sharedPtr, etc.
├── src/                    # Source files
│   ├── server/             # Event loop, epoll wrapper, Server/Client implementations
│   ├── http/               # HTTP parser, RequestHandler, Response
│   ├── cgi/                # CGI execution & plumbing
│   └── utils/              # Logger, RingBuffer implementations
├── configs/                # Default configuration and error pages
├── docs/                   # Design & implementation notes (this folder)
├── test/                   # Manual test pages and CGI scripts
└── www/                    # Example static files (document root)
```

Key mappings (where to look in the codebase):

- Event loop and core server flow: `include/server/EventLoop.hpp` and `src/server/EventLoop.cpp`.
- Epoll and file descriptor management: `include/server/Epoll.hpp`, `include/server/FdManager.hpp`, `src/server/Epoll.cpp`, `src/server/FdManager.cpp`.
- Server bootstrap and socket handling: `include/server/Server.hpp` and `src/server/Server.cpp`.
- Client connection lifecycle: `include/server/Client.hpp` and `src/server/Client.cpp`.
- HTTP parsing and request routing: `include/http/HTTPParser.hpp`, `include/http/RequestHandler.hpp`, and `src/http/`.
- CGI handling: `include/cgi/CGIHandler.hpp` and `src/cgi/CGIHandler.cpp`.
- Configuration: `include/Config/ConfigParser.hpp` and `src/Config/ConfigParser.cpp`.
- Utilities: `include/utils/*` and `src/utils/*` (Logger, RingBuffer, sharedPtr).

Design notes
- The server is event-driven and uses Linux `epoll()` for readiness notifications.
- The implementation uses level-triggered epoll by default. See `docs/LEVEL_TRIGGERED_IMPLEMENTATION.md` for rationale and details.
- Handlers (Client, Server, etc.) implement an EventHandler-like interface and register/unregister with `FdManager`.

Navigation tips
- If you want to trace a request: start with `src/server/Client.cpp` → `src/http/RequestHandler.cpp` → `src/http/Response.cpp`.
- For shutdown and signal handling, inspect `src/server/EventLoop.cpp` and the global `g_shutdown` usage.

If anything in the repo layout drifts from this description, update this document so other contributors can find code quickly.

````