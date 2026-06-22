# Issues — content_service

## Potential Bugs

### Concurrency Issues

#### Issue 1: Shared `ContentManifest`/`ContentScanner` state mutated without synchronization

- **Location:** `include/oink_judge/content_service/content_scanner.h:21` (`last_full_rescan_`), `src/content_manifest.cpp:61-63` (`updateManifest`)
- **Description:** `ManifestStorage::getManifest` hands out `const ContentManifest&` to concurrent gRPC handler coroutines. `ContentManifest::updateManifest` is `const` but mutates the owned `ContentScanner` through `content_scanner_` (a `unique_ptr`), including `last_full_rescan_`, and writes `manifest.json` to disk. Two concurrent `toString()`/`toJson()` calls on the same manifest produce an unsynchronized data race on `last_full_rescan_` and racing writes to the same `manifest.json` file (a reader may also see a partially written file via `toString`).
- **Suggested fix:** Add a `mutable std::mutex` in `ContentManifest` guarding `updateManifest` (scan + store), or serialize per-manifest access in `ManifestStorage`. Write `manifest.json` atomically (write to temp file + rename).

---

### Unchecked JSON Access

#### Issue 2: `compareManifests` dereferences missing keys in file entries

- **Location:** `src/content_manifest.cpp:83-85`
- **Description:** `old_file_info["sha256"]`, `new_file_info["sha256"]` and the `"permissions"` accesses use `operator[]` on `const json`, which is undefined behavior in nlohmann-json when the key is absent. The new manifest comes from the network (`ContentStorage::ensureContentExists` passes the server response), so a malformed or older-schema manifest can crash the client.
- **Suggested fix:** Use `.value("sha256", "")`/`.contains()` guards and define explicit behavior for entries missing required fields (e.g. treat as `MODIFIED`).

---

### Exception Safety

#### Issue 3: `ContentServiceChannelStub::getManifest` can throw instead of returning `tl::unexpected`

- **Location:** `src/client/content_service_stub.cpp:52`
- **Description:** After a successful RPC, `json::parse(manifest_stream.str())` throws `json::parse_error` if the server streams malformed JSON (or a chunk-reassembly bug occurs). This escapes the `tl::expected<json, grpc::Status>` error contract and propagates an exception out of the coroutine, which callers handling only the `expected` error channel will not anticipate.
- **Suggested fix:** Wrap the parse in try/catch and return `tl::unexpected(grpc::Status(grpc::StatusCode::INTERNAL, ...))`, or change the return type to carry parse failures explicitly.

---

### Security Issues

#### Issue 4: `polygon_converter.cpp` — command injection via `std::system()`

- **Location:** `src/problem_package_converter/polygon_converter.cpp:40-41`
- **Description:** `path_to_checker` is read from the `problem.xml` attribute value and `path_to_package` comes from a CLI argument. Both are interpolated directly into a shell command string passed to `std::system()`. A path containing shell metacharacters (spaces, `;`, `$(...)`, backticks, etc.) allows execution of arbitrary shell commands. Additionally, `path_to_checker` is not validated to be within `path_to_package`, so a crafted XML attribute such as `path="../../etc/passwd"` can reference arbitrary host files.
- **Suggested fix:** Use `execv`/`posix_spawn` with an explicit argument array instead of `std::system()`. Validate that the resolved checker path is within `path_to_package` before using it.

---

## Architecture Issues

### Testability

#### Issue 5: Singleton design prevents unit testing

- **Location:** `include/oink_judge/content_service/client/content_storage.h`, `include/oink_judge/content_service/manifest_storage.h`
- **Description:** Both `ManifestStorage` and `ContentStorage` are singletons accessed directly. There is no way to inject a test double, making it impossible to unit-test `ContentStorage` without a live gRPC server and real filesystem layout matching production config (see `tests_to_add.md` Cases 30–34, which are blocked on this).
- **Suggested fix:** Introduce constructor injection for these dependencies, and have callers receive them by reference or pointer rather than calling `::instance()` directly.

---

### Code Duplication

#### Issue 6: `getPathToContentDirectory` / `storedManifestToJson` duplicated across translation units

- **Location:** `src/content_manifest.cpp:18-35`, `src/content_scanner.cpp:19-36`
- **Description:** Both files define identical anonymous-namespace copies of these helpers. Divergence is already visible: `content_scanner.cpp` logs under the module name `"content_manifest"`, so scanner errors are misattributed in logs. Any future fix (e.g. atomic manifest reads) must be applied twice.
- **Suggested fix:** Extract the helpers into a shared internal header/source (e.g. `src/manifest_io.h`) used by both, and fix the logger module name.

#### Issue 7: 64KB chunk-streaming loops duplicated four times

- **Location:** `src/client/content_service_stub.cpp:102-118, 146-162`; `src/server/content_service.cpp:108-124, 143-160` (plus the read loops in `createFileHandler`/`updateFileHandler`)
- **Description:** The client `createFile`/`updateFile` methods and the server `getManifestHandler`/`getFileHandler` each contain a near-identical buffer/chunk loop, differing only in the request/response message type. The server `createFileHandler` and `updateFileHandler` bodies are likewise almost line-for-line identical. A bug fix in the chunking logic (e.g. the redundant `bytes_read == 0` / `bytes_read > 0` double check) must be repeated in every copy.
- **Suggested fix:** Extract templated helpers, e.g. `streamStringAsChunks<Rpc, Msg>(...)` for writers and a shared `readChunksToString` for the two upload handlers.
