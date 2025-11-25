# Level-Triggered Event Loop Implementation - Final Summary

## Changes Made

Successfully converted the event loop implementation from Edge-Triggered (ET) to Level-Triggered (LT) mode with proper error handling and best practices.

## Key Modifications

### 1. **Client.cpp** - Simplified I/O Handling
- **Removed**: errno.h include and EAGAIN/EWOULDBLOCK checks
- **Simplified**: readRequest() - any recv() error < 0 is treated as real error
- **Simplified**: _sendResponseChunk() - any send() error < 0 is treated as real error
- **Added**: Partial write tracking with warning logs
- **Changed**: All epoll modifications use EPOLLIN/EPOLLOUT (no EPOLLET flag)

**Benefit**: Simpler, more maintainable code. Level-triggered mode automatically retries when data is available.

### 2. **Server.cpp** - Level-Triggered Client Registration
- **Changed**: Client registration from `EPOLLIN | EPOLLET` to `EPOLLIN`
- **Maintained**: SO_REUSEADDR, non-blocking mode, error isolation

**Benefit**: More reliable connection handling, won't miss events.

### 3. **EventLoop.cpp** - Enhanced Event Processing
- **Added**: csignal include for sig_atomic_t
- **Added**: 1-second timeout in epoll_wait() for signal responsiveness
- **Added**: Error-first processing (ERROR → READ → WRITE)
- **Added**: Handler existence validation between events
- **Enhanced**: Exception handling with cleanup

**Benefit**: Robust event processing that handles edge cases gracefully.

### 4. **EventHandler.hpp** - Fixed Circular Dependency
- **Changed**: Forward declaration for FdManager instead of include
- **Changed**: Constructor initialization order (_fd_manager before _config)
- **Added**: inline keyword for constructor to prevent multiple definition

**Benefit**: Proper header organization, faster compilation.

### 5. **Routing.hpp & Related** - Fixed Naming Conflict
- **Changed**: `Server` → `ServerConfig` throughout routing code
- **Fixed**: Routing.hpp, Routing.cpp, handleRequest.cpp

**Benefit**: Eliminated naming conflict between config Server and event Server class.

### 6. **Documentation** - Updated Architecture Docs
- **Updated**: EVENT_LOOP_ARCHITECTURE.md for level-triggered approach
- **Updated**: IMPLEMENTATION_SUMMARY.md with new implementation details
- **Removed**: References to errno checking and EAGAIN handling
- **Added**: Level-triggered benefits and implementation details

## Level-Triggered vs Edge-Triggered Comparison

### Level-Triggered (Current Implementation)

**Advantages:**
- ✅ Simpler to implement correctly
- ✅ More forgiving - won't miss events
- ✅ No need to read/write until EAGAIN
- ✅ Easier to debug
- ✅ Default epoll behavior (well-tested)
- ✅ Reliable and predictable

**Trade-offs:**
- More epoll_wait() calls (still efficient)
- May get multiple notifications for same event

### Edge-Triggered (Previous Approach)

**Advantages:**
- Fewer epoll_wait() calls
- Better for very high connection counts (10k+)

**Disadvantages:**
- Must read until EAGAIN (can miss data otherwise)
- Must write until EAGAIN
- More complex error handling
- Easier to introduce bugs
# Level-triggered implementation — concise explanation

This repository uses level-triggered epoll notifications for clarity and reliability. This document summarizes the rationale and where to look in the codebase for the implementation.

Why level-triggered (LT)?
- Simpler read/write logic — kernel will keep reporting readiness until condition is cleared.
- Lower chance of subtle bugs introduced by missing EAGAIN loops required by edge-triggered (ET).
- Easier to debug and maintain during development.

Where LT behavior is implemented
- Client read/write handling: `src/server/Client.cpp` — onReadable/onWritable, partial write tracking.
- Client registration: `src/server/Server.cpp` — register clients with `EPOLLIN` and switch to `EPOLLOUT` when sending.
- Event dispatch and safety: `src/server/EventLoop.cpp` and `include/server/FdManager.hpp`.

Implementation notes
- The code treats recv/send returning negative values as real errors (in LT mode the kernel will re-notify when data is present again).
- Partial writes are detected and tracked; remaining bytes are sent on the next EPOLLOUT notification.
- Handlers validate existence through `FdManager::exists()` between events to avoid processing after deletion.

Trade-offs vs ET
- ET can be more efficient under extreme loads (fewer syscalls) but requires reading/writing loops until EAGAIN and careful errno handling.
- LT is slightly more syscall-heavy but safer and easier to reason about for a general-purpose HTTP server.

Testing checklist
- Verify basic requests (static files from `www/`).
- Test large responses to exercise partial write logic.
- Test CGI scripts under `test/cgi_scripts/`.
- Verify graceful shutdown (send SIGINT and ensure cleanup).

If you need higher-performance ET semantics later, the change points are:
1. `src/server/Client.cpp` — implement read/write loops until EAGAIN.
2. Registration flags — add `EPOLLET` during `FdManager::add`/`Server::accept`.
3. Careful errno handling and more aggressive zero-copy I/O (optional).
