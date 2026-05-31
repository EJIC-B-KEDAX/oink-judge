# Socket Module — Problems

## 1. `ConnectionStorage::removeConnection` erases too many elements

**File:** `src/connection_storage.cpp`

```cpp
connections_.erase(it, connections_.end());
```

This erases from the found element **to the end of the vector**, removing all subsequent connections instead of just the target one.

**Fix:** Use the single-iterator overload: `connections_.erase(it);`

Fixed

---

## 2. No message size limit — denial-of-service via memory exhaustion

**Files:** `src/async_server.cpp`, `src/sessions/ssl_session.cpp`

Both `AsyncServer::accept()` and `SSLSession::receiveMessage()` read a `uint64_t` length from the network and allocate a `std::string` of that size without any upper-bound check. A malicious client can send a length like `0xFFFFFFFFFFFFFFFF` and crash the server with an allocation failure.

**Fix:** Define a maximum allowed message size (e.g. 16 MB) and reject messages exceeding it.

---

## 3. `hton64` uses signed cast — undefined/implementation-defined behavior

**File:** `src/byte_order.cpp`

```cpp
return static_cast<int64_t>(htonl(val & 0xFFFFFFFF)) << 32 | ...
```

Left-shifting a signed `int64_t` whose high bit may be set is undefined behavior (before C++20) or implementation-defined. This should be `static_cast<uint64_t>`.

**Fix:** Replace `static_cast<int64_t>` with `static_cast<uint64_t>` in both `hton64` and `ntoh64`.

Fixed

---

## 4. `hton64` / `ntoh64` are in the global namespace

**File:** `include/oink_judge/socket/byte_order.h`

These functions are declared outside any namespace, which is inconsistent with the rest of the module and risks name collisions with other libraries or system headers.

**Fix:** Move them into `oink_judge::socket` or at least a project-specific namespace.

---

## 5. `ProtocolBase::getSession` — TOCTOU race on `weak_ptr`

**File:** `src/protocols/protocol_base.cpp`

```cpp
if (session_.expired()) { throw ... }
return session_.lock();
```

Between the `expired()` check and the `lock()` call, the session may be destroyed by another thread. The `lock()` could then return `nullptr`.

**Fix:** Call `lock()` first, then check the result for `nullptr`:

```cpp
auto sp = session_.lock();
if (!sp) throw ...;
return sp;
```

---

## 6. `ProtocolWithRequests::requestInternal` — non-atomic static counter

**File:** `src/protocols/protocol_with_requests.cpp`

```cpp
static uint64_t request_id_counter = 0;
uint64_t request_id = ++request_id_counter;
```

`++request_id_counter` is a data race if called from multiple threads. Two requests could receive the same ID.

**Fix:** Use `static std::atomic<uint64_t> request_id_counter{0};`

---

## 7. `SSLSession::is_sending_` and `message_queue_` — no synchronization

**File:** `src/sessions/ssl_session.cpp`, `include/oink_judge/socket/sessions/ssl_session.h`

`is_sending_` is a plain `bool` and `message_queue_` is a plain `std::queue`, both accessed from potentially concurrent coroutines (every `sendMessage` caller and the `sendLoop` coroutine). This is a data race.

**Fix:** Either protect access with a mutex/strand, or use `std::atomic<bool>` for the flag and serialize access to the queue through a strand.

---

## 8. Infinite retry loops in `connection_protocol.cpp`

**File:** `src/connection_protocol.cpp`

`asyncConnectToTheEndpoint` ↔ `asyncScheduleConnectToTheEndpoint` and `connectToTheEndpoint` ↔ `scheduleConnectToTheEndpoint` call each other in an unbounded mutual recursion. If the remote endpoint is permanently unreachable, this retries forever with only a 1-second delay, consuming resources (stack frames for the sync version, coroutine frames for the async version).

**Fix:** Add a maximum retry count or exponential backoff, and return an error or `nullptr` when exhausted.

---

## 9. `asyncScheduleConnectToTheEndpoint` — double reconnect attempt on timer error

**File:** `src/connection_protocol.cpp`

```cpp
try {
    co_await timer.async_wait(...);
    co_return co_await asyncConnectToTheEndpoint(...);
} catch (...) {
    logger::logError(...);
}
co_return co_await asyncConnectToTheEndpoint(...);
```

If the timer wait throws, the catch block logs the error but then falls through to **another** `asyncConnectToTheEndpoint` call at the end of the function, causing a duplicate connection attempt.

**Fix:** Either `co_return` from the catch block or remove the trailing call after the try-catch.

---

## 10. `connectToTheEndpoint` blocks the I/O thread with `run_one()` loop

**File:** `src/connection_protocol.cpp`

```cpp
while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    io_context.run_one();
}
```

This spins on the calling thread, processing events from the shared `io_context`. It can cause reentrancy issues (handlers running inside `run_one` that were not expected) and prevents other work from executing on that thread.

**Fix:** Use a dedicated strand or a separate `io_context` for blocking operations, or restructure to use only coroutines.

---

## 11. `connectToTheEndpoint` creates socket as `shared_ptr` then moves from it

**File:** `src/connection_protocol.cpp`

```cpp
auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
// ...
auto ptr = SessionFactory::instance().create(session_type, std::move(*socket));
```

A `shared_ptr` is used for no reason (the socket is local). After `std::move(*socket)`, the `shared_ptr` still holds a moved-from socket object. The `socket->close()` in the catch block then operates on a moved-from object, which is undefined behavior for Boost sockets.

**Fix:** Use a local `tcp::socket` object directly instead of `shared_ptr`.

---

## 12. `BoostSSLContext::server()`/`client()` — non-thread-safe lazy initialization

**File:** `src/boost_ssl_context.cpp`

```cpp
static bool initialized = false;
if (!initialized) {
    // ... setup ...
    initialized = true;
}
```

While C++11 guarantees thread-safe initialization of `static` locals, subsequent reads and writes to `initialized` are **not** synchronized. Two threads calling `server()` concurrently could both see `initialized == false`.

**Fix:** Perform the SSL setup inside the static variable's own initialization (e.g. use a helper function that returns the fully initialized context), or protect with `std::call_once`.

---

## 13. `BoostSSLContext` forces TLS 1.2 — no TLS 1.3 support

**File:** `src/boost_ssl_context.cpp`

```cpp
static boost::asio::ssl::context server_context(boost::asio::ssl::context::tlsv12_server);
```

`tlsv12_server` restricts connections to exactly TLS 1.2. TLS 1.3 is not supported. Modern deployments should support TLS 1.3.

**Fix:** Use `boost::asio::ssl::context::tls_server` / `tls_client` and set the minimum version to TLS 1.2 via `SSL_CTX_set_min_proto_version(ctx.native_handle(), TLS1_2_VERSION)`.

---

## 14. Port type is `short` — too narrow for valid port numbers

**Files:** `include/oink_judge/socket/async_server.h`, `include/oink_judge/socket/client_config_utils.h`, `include/oink_judge/socket/connection_protocol.h`, `src/client_config_utils.cpp`

`short` is typically 16-bit signed (max 32767), but valid TCP ports go up to 65535. Additionally, `getServerPort` returns `std::optional<int>` but the result is assigned to `std::optional<short>` — a narrowing conversion.

**Fix:** Use `uint16_t` (or `unsigned short` / `int`) consistently for port numbers throughout the module.

---

## 15. `SimpleConnectionHandler::newConnection` — unhandled JSON parse exceptions

**File:** `src/simple_connection_handler.cpp`

```cpp
json parsed_message = json::parse(start_message);
std::string connection_type = parsed_message.at("connection_type");
```

If `start_message` is not valid JSON or lacks the `"connection_type"` key, these calls throw exceptions that are never caught. This would crash the accept loop.

**Fix:** Wrap in a try-catch, log the error, and close the socket gracefully.

---

## 16. `AuthRequiredProtocol` — plain-text auth token comparison (timing attack)

**File:** `src/protocols/auth_required_protocol.cpp`

```cpp
if (message == auth_token_) {
```

The `==` operator on `std::string` short-circuits on the first differing byte. An attacker can measure response times to guess the token byte-by-byte.

**Fix:** Use a constant-time comparison function (e.g. `CRYPTO_memcmp` from OpenSSL, which is already linked).

---

## 17. Duplicated auth token path parsing logic

**Files:** `src/protocols/auth_required_protocol.cpp`, `src/protocols/authorizing_protocol.cpp`

The registration functions for `AuthRequiredProtocol` and `AuthorizingProtocol` contain identical credential path parsing code (~20 lines). This violates DRY and doubles the maintenance burden.

**Fix:** Extract the parsing logic into a shared utility function, e.g. `resolveCredentialPath(const std::string& path) -> std::string`.

---

## 18. `PingingProtocol::pingLoop` / `waitForPong` — dangling `this` capture

**File:** `src/protocols/pinging_protocol.cpp`

Timer callbacks capture `this` (the `PingingProtocol` pointer) and a `shared_ptr<Session>`. The `Session` is kept alive, but the `Protocol` itself is owned by the `Session` via `unique_ptr`. If the session is closed and destroyed from another path, or if the protocol is replaced, `this` becomes dangling while the timer callback is still pending.

**Fix:** Capture a reference-counted handle to the protocol (e.g. via `shared_from_this`), or cancel the timers in the destructor and ensure the protocol outlives all pending callbacks.

---

## 19. `SSLSession::~SSLSession` calls `close()` which calls `closeSession()` on protocol

**File:** `src/sessions/ssl_session.cpp`

The destructor calls `close()`, which may access `accessProtocol().closeSession()`. At destruction time, `shared_from_this()` is invalid (the control block's weak count may have already been decremented). If `closeSession()` triggers any code that needs `shared_from_this()`, it will throw. Additionally, calling virtual functions (`accessProtocol()`) in a destructor is fragile.

**Fix:** Ensure `close()` is called explicitly before destruction (e.g. in a `shutdown()` method), or make the destructor only close the underlying socket without invoking protocol callbacks.

---

## 20. `SSLSession::sendLoop` — remaining queued messages' callbacks are never invoked on error

**File:** `src/sessions/ssl_session.cpp`

When `sendLoop` encounters a write error, it invokes the callback for the current message, calls `close()`, and returns. All other messages still in `message_queue_` have their callbacks silently dropped. The coroutines waiting on those `sendMessage` calls will hang forever.

**Fix:** After closing, drain the remaining queue and invoke each callback with an error code.

---

## 21. `BoostIOContext` / `BoostSSLContext` destructors declared but never defined

**Files:** `include/oink_judge/socket/boost_io_context.h`, `include/oink_judge/socket/boost_ssl_context.h`

Both classes declare `~BoostIOContext();` / `~BoostSSLContext();` in the header, but no definition exists in the corresponding `.cpp` files. This works only because no instances of these classes are ever constructed. If someone accidentally creates one, it will fail to link.

**Fix:** Change to `~BoostIOContext() = default;` (and similarly for `BoostSSLContext`) in the header.

---

## 22. Single global `io_context` for all I/O

**File:** `src/boost_io_context.cpp`

A single `io_context` instance is shared across the entire application. This limits concurrency to a single event-processing thread and makes it impossible to scale I/O processing across multiple cores.

**Fix:** Consider using a thread pool with `io_context::run()` on multiple threads, or allow creating multiple `io_context` instances for different subsystems.

---

## 23. `AsyncServer::accept()` — rapid retry on persistent accept errors

**File:** `src/async_server.cpp`

When `async_accept` throws (outer catch), the function immediately spawns another `accept()` without any delay. If the error is persistent (e.g. too many open file descriptors), this becomes a busy loop consuming CPU.

**Fix:** Add a short delay (e.g. via `steady_timer`) before retrying after an accept error.

---

## 24. `ProtocolWithRequests::pending_requests_callbacks_` — no thread safety

**File:** `src/protocols/protocol_with_requests.cpp`

The `pending_requests_callbacks_` map is accessed from `requestInternal` (send path) and from whatever processes responses (receive path). If these run on different threads or strands, concurrent access is a data race leading to undefined behavior.

**Fix:** Protect with a mutex or ensure all access occurs on the same strand.

---

## 25. `AuthRequiredProtocol::start` does not call `ProtocolDecorator::start`

**File:** `src/protocols/auth_required_protocol.cpp`

`AuthRequiredProtocol::start()` spawns a `receiveMessage()` and returns without calling the base `ProtocolDecorator::start()`. The base start is deferred until after authorization. This means the inner protocol's `start()` is not invoked until after auth succeeds, which may leave the inner protocol in an inconsistent state if certain initialization is expected at `start()` time.

**Fix:** Document this explicitly as intended behavior, or restructure so that inner protocol initialization that doesn't depend on auth happens at `start()` time.

---

# Architectural Problems (Sessions & Protocols)

## 26. No centralized receive loop — fragile, manual re-triggering

`SSLSession::receiveMessage()` reads **one** message from the network and calls `protocol->receiveMessage(message)`. There is no loop. Instead, individual protocols are responsible for spawning the next `receiveMessage()` on the session:

- `PingingProtocol::receiveMessage` spawns next read after `"pong"`
- `PongingProtocol::receiveMessage` spawns next read after `"ping"`
- `AuthRequiredProtocol::start` spawns the first read

For non-special messages (delegated to inner protocols), **nobody** spawns the next read unless the innermost protocol does it. If any protocol in the decorator chain forgets, the session silently stops receiving. Receive loop ownership is scattered across the codebase instead of being in one place.

**Fix:** Move the receive loop into `SSLSession::start()` — a `while(is_open)` coroutine loop that reads messages and dispatches them to the protocol. Protocols should never need to manually re-trigger reads.

---

## 27. Circular dependency between Session and Protocol

```
Session ──owns (unique_ptr)──▶ Protocol
Protocol ──weak_ptr──────────▶ Session
```

`Protocol::sendMessage()` calls `getSession()->sendMessage()`, and `Session::receiveMessage()` calls `protocol->receiveMessage()`. They are tightly coupled and mutually dependent. This makes it hard to test, reason about lifetimes, or reuse either component independently.

**Fix:** Consider passing the session (or a narrower "transport" interface) as a parameter to protocol methods instead of storing it:

```cpp
virtual auto receiveMessage(std::string message, Session& session) -> awaitable<void> = 0;
```

---

## 28. `sendMessage` / `receiveMessage` naming asymmetry across layers


| Layer        | `sendMessage`                                 | `receiveMessage`                                          |
| ------------ | --------------------------------------------- | --------------------------------------------------------- |
| **Session**  | Takes a string, writes to network             | Takes no args, reads from network, dispatches to protocol |
| **Protocol** | Takes a string, calls`session->sendMessage()` | Takes a string (already read), processes it               |

`receiveMessage` means completely different things at the two layers — "read from wire" vs "handle already-read data". This is confusing, especially in decorator chains.

**Fix:** Rename the protocol method to `handleMessage(std::string message)` or `onMessageReceived(std::string message)` to clarify it processes an already-received message.

---

## 29. `start()` semantics are inconsistent across protocols

- `SSLSession::start()` → does SSL handshake, then calls `protocol->start()`
- `ProtocolDecorator::start()` → forwards to inner protocol
- `AuthRequiredProtocol::start()` → does **not** call inner `start()`, spawns a receive instead. Inner `start()` is deferred until after successful auth.
- `PingingProtocol::start()` → calls inner `start()`, **then** starts ping loop

The `start()` contract is undefined — some protocols forward, some don't, some add pre/post behavior. A developer adding a new decorator has no way to know what the contract is.

**Fix:** Split into separate lifecycle phases with clear contracts:

```cpp
virtual auto initialize() -> void;       // setup, always called, always forwarded
virtual auto onConnected(std::string start_message) -> awaitable<void>; // post-handshake logic
```

---

## 30. `closeSession()` vs `close()` — bidirectional close is tangled

- `Session::close()` closes the socket **and** calls `protocol->closeSession()`
- `Protocol::closeSession()` can call `getSession()->close()` (some protocols do)
- `SSLSession::~SSLSession()` calls `close()`

This creates potential for infinite recursion (`close()` → `closeSession()` → `close()`...). It is only prevented by the `is_open()` guard in `SSLSession::close()`, which is fragile. Destruction order issues also apply (see #19).

**Fix:** Separate "close the transport" from "notify the protocol that the session ended":

```cpp
// Session side
auto close() -> void;              // closes socket only, no protocol callback
auto shutdown() -> void;           // close + notify protocol

// Protocol side
auto onSessionClosed() -> void;    // notification only, must NOT call session->close()
```

---

## 31. `ProtocolWithRequests` can only be a leaf — not composable via decoration

`ProtocolWithRequests` extends `ProtocolBase` (the leaf base class), not `ProtocolDecorator`. This means request-tracking behavior **cannot** be added to an arbitrary protocol via decoration — it must always be the innermost protocol.

If you wanted e.g. a protocol that has both custom message handling **and** request tracking, you'd have to inherit from `ProtocolWithRequests` directly.

**Fix:** Make request tracking a decorator too, so it can be composed freely:

```cpp
class RequestTrackingProtocol : public ProtocolDecorator { ... };
```

---

## 32. The `request()` mechanism bypasses the decorator chain

`Session::request()` calls `Session::requestInternal()`, which calls `Protocol::requestInternal()`. In the decorator chain, `ProtocolDecorator::requestInternal()` forwards to the inner protocol. But `ProtocolWithRequests::requestInternal()` calls `sendMessage()`, which goes through `ProtocolBase::sendMessage()` → `session->sendMessage()`.

This means the request's **outgoing** message skips the decorator chain (it goes straight to the session), but the **incoming** response goes through the full decorator chain via `receiveMessage`. This asymmetry can cause subtle bugs — e.g., if a decorator modifies outgoing messages, it won't see request messages.

**Fix:** Route request sends through the full protocol chain, or document this explicitly as intentional.
