# Tests to Add — content_service

All previously listed cases are now implemented in `tests/`, except the `ContentStorage` block below.

## `ContentStorage` — Sync Logic (requires stub injection)

> Blocked on `issues.md` Issue "Singleton design prevents unit testing": `ContentStorage` needs a way to inject a mock `ContentServiceStub` (constructor injection or a test-only setter). Once available, add the following with a mock stub recording calls and serving canned manifests/files.

### Case 1: `ensureContentExists` downloads added and modified files with permissions

- **Function / Class:** `ContentStorage::ensureContentExists`
- **Scenario:** Mock server manifest contains a file absent locally (ADDED) and one with a different sha256 (MODIFIED); run `ensureContentExists`.
- **Why it matters:** Core client sync flow — `getFile` must be called for both change types, content stored under the local content path, and permissions applied from the server manifest.
- **Expected behavior:** Both files exist locally with the served content and the manifest's permissions.

---

### Case 2: `ensureContentExists` applies attribute-only changes and removals

- **Function / Class:** `ContentStorage::ensureContentExists`
- **Scenario:** Server manifest differs only in `permissions` for one file (ATTRIBUTES_CHANGED) and omits another local file (REMOVED).
- **Why it matters:** The ATTRIBUTES_CHANGED branch must not re-download content; the REMOVED branch must delete local files — neither path is exercised anywhere.
- **Expected behavior:** Permissions updated without a `getFile` call; the removed file is deleted locally.

---

### Case 3: `ensureContentExists` with no changes performs no file operations

- **Function / Class:** `ContentStorage::ensureContentExists`
- **Scenario:** Server manifest identical to the local one.
- **Why it matters:** The early `co_return` on empty changes avoids needless I/O; a regression would re-download all content on every call.
- **Expected behavior:** Only `getManifest` is called on the stub; local files untouched.

---

### Case 4: `updateContentOnServer` pushes added, modified, and removed files

- **Function / Class:** `ContentStorage::updateContentOnServer`
- **Scenario:** Local content has one file unknown to the server manifest, one with a different sha256, and the server manifest has one file missing locally.
- **Why it matters:** Verifies correct mapping: ADDED→`createFile`, MODIFIED→`updateFile`, REMOVED→`deleteFile`, with file contents loaded from disk. Note the comparison direction is inverted relative to `ensureContentExists`.
- **Expected behavior:** Mock stub records exactly one `createFile`, one `updateFile`, one `deleteFile` with correct paths and contents.

---

### Case 5: `updateContentOnServer` throws when local content path is missing

- **Function / Class:** `ContentStorage::updateContentOnServer`
- **Scenario:** Call for a `content_id` with no local directory.
- **Why it matters:** Explicit guard at the top of the function; the error message includes the path and must be raised before any RPC.
- **Expected behavior:** Throws `std::runtime_error` ("Content path does not exist..."); no stub calls made.
