# Test Generation Guide for oink-judge

## Framework

All tests use **Google Test** (`gtest`). Do **not** write custom `main()` functions or manual assertion helpers — use `TEST()` and `TEST_F()` macros exclusively. The `GTest::gtest_main` target provides the entry point automatically.

## Project Conventions

- **C++ standard:** C++20
- **Return type syntax:** Trailing return types (`auto foo() -> ReturnType`)
- **Include style:**
  - Same-module headers: `#include "oink_judge/<module>/header.h"`
  - Cross-module headers: `#include <oink_judge/<module>/header.h>`
  - gtest: `#include <gtest/gtest.h>`
- **Namespace usage:** `using namespace oink_judge::<module>;` at file scope
- **Filesystem alias:** `namespace fs = std::filesystem;` (only when needed)
- See also: [Code Style & Naming Conventions](codestyle.md)

## File and Directory Layout

```
modules/<module>/
├── include/oink_judge/<module>/   # Public headers
├── src/                           # Implementation
├── tests/
│   ├── CMakeLists.txt             # Test build config
│   ├── test_<feature>.cpp         # Test source files
│   └── resources/
│       └── test_<feature>/        # Resource directory per test file
│           ├── good_config.json
│           └── ...
```

Each test file gets its own resource subdirectory named `test_<feature>` under `resources/`.

## CMake Registration

### Test CMakeLists.txt (`modules/<module>/tests/CMakeLists.txt`)

```cmake
include(AddModuleTest)
add_module_test(<test_target_name> <test_source>.cpp oink_judge::<module>)

# Only if the test uses resource files:
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
```

- **Target naming:** `oink_judge_<descriptive_name>_tests` (e.g., `oink_judge_config_tests`, `oink_judge_logger_config_tests`)
- **Link target:** Use the module's alias target (e.g., `oink_judge::config`, `oink_judge::logger`)
- Multiple test files in one module each get their own `add_module_test()` call

### How `add_module_test` works

```cmake
function(add_module_test TEST_NAME TEST_SOURCE LINK_TARGET)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${LINK_TARGET} GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endfunction()
```

It links `GTest::gtest_main` automatically, so tests must **not** define `main()`.

### Module CMakeLists.txt

If the module doesn't already include its tests directory, add:

```cmake
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

## Available Module Targets


| Alias Target                         | Module                   |
| ------------------------------------ | ------------------------ |
| `oink_judge::logger`                 | Logger                   |
| `oink_judge::utils`                  | Shared utilities         |
| `oink_judge::factory`                | Factory (INTERFACE)      |
| `oink_judge::config`                 | Configuration            |
| `oink_judge::plugin_manager`         | Plugin manager           |
| `oink_judge::database`               | Database                 |
| `oink_judge::socket`                 | Socket                   |
| `oink_judge::content_service_common` | Content service (common) |
| `oink_judge::content_service_client` | Content service (client) |
| `oink_judge::content_service_server` | Content service (server) |
| `oink_judge::dispatcher`             | Dispatcher               |
| `oink_judge::auth_service`           | Auth service             |

## Writing a Test File

### Test Fixture Pattern (preferred)

Use `TEST_F` with a fixture class when tests share setup logic (e.g., loading config files, initializing resources).

```cpp
#include "oink_judge/<module>/<header>.h"

#include <gtest/gtest.h>

using namespace oink_judge::<module>;

class FeatureTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = std::filesystem::path("resources") / "test_<feature>";
        // Common setup: load configs, initialize objects, etc.
    }

    auto getResourcesPath() -> const std::filesystem::path& { return resources_; }

  private:
    std::filesystem::path resources_;
};

TEST_F(FeatureTest, DescriptiveTestName) {
    // Arrange / Act / Assert
    auto result = someFunction();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, expectedValue);
}
```

### Simple Tests (no shared setup)

Use `TEST()` when tests are independent and don't need a fixture.

```cpp
#include "oink_judge/<module>/<header>.h"

#include <gtest/gtest.h>

using namespace oink_judge::<module>;

TEST(FeatureTest, BasicBehavior) {
    EXPECT_EQ(compute(2, 3), 5);
}
```

## Assertion Guidelines

- **`ASSERT_*`** — Fatal: stops the test on failure. Use when subsequent assertions depend on this one (e.g., checking `.has_value()` before dereferencing).
- **`EXPECT_*`** — Non-fatal: continues the test on failure. Use for independent checks.

Common assertions:


| Assertion                      | Purpose                             |
| ------------------------------ | ----------------------------------- |
| `EXPECT_EQ(a, b)`              | Equality                            |
| `EXPECT_NE(a, b)`              | Inequality                          |
| `EXPECT_TRUE(expr)`            | Boolean true                        |
| `EXPECT_FALSE(expr)`           | Boolean false                       |
| `ASSERT_TRUE(opt.has_value())` | Guard before dereferencing optional |
| `EXPECT_ANY_THROW(expr)`       | Expects any exception               |
| `EXPECT_THROW(expr, ExType)`   | Expects specific exception type     |

### Pattern for `std::optional` results

```cpp
auto result = getOptionalValue();
ASSERT_TRUE(result.has_value());   // Fatal — stop if no value
EXPECT_EQ(*result, expected);      // Non-fatal — check the value
```

## Test Resource Files

- Place JSON configs and other test data in `tests/resources/test_<feature>/`
- Access them in tests via relative path: `std::filesystem::path("resources") / "test_<feature>"`
- The `file(COPY ...)` directive in CMakeLists.txt ensures they are available at test runtime
- Use descriptive filenames: `good_config.json`, `no_logger.json`, `bad_malformed.json`

### Example resource JSON

```json
{
    "logger": {
        "output_stream": "stdout",
        "log_levels": {
            "moduleA": 2,
            "default": 3
        }
    }
}
```

## Test Naming

- **Fixture class:** `<Feature>Test` (e.g., `ConfigTest`, `LoggerConfigTest`)
- **Test name:** Descriptive, PascalCase, states expected behavior (e.g., `DirectoryPathReturnsCorrectPath`, `MissingConfigThrows`, `OutputStreamReturnsStdout`)
- **Test target:** `oink_judge_<feature>_tests` (e.g., `oink_judge_config_tests`)

## Helper Methods in Fixtures

When multiple tests need to reconfigure state (e.g., loading a different config file), add a static or protected helper to the fixture:

```cpp
class LoggerConfigTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = std::filesystem::path("resources") / "test_logger_config";
        LoggerConfigTest::loadConfig(resources_ / "good_config.json", resources_ / "good_credentials.json");
    }

    auto getResourcesPath() -> const std::filesystem::path& { return resources_; }

    static auto loadConfig(const std::filesystem::path& config, const std::filesystem::path& credentials) -> void {
        Config::setConfigFilePath(config);
        Config::setCredentialsFilePath(credentials);
        Config::reloadData();
    }

  private:
    std::filesystem::path resources_;
};
```

## Complete Example

```cpp
#include "oink_judge/config/common_utils.h"
#include "oink_judge/config/config.h"

#include <gtest/gtest.h>

using namespace oink_judge::config;

class ConfigTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = std::filesystem::path("resources") / "test_config";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

    auto getResourcesPath() -> const std::filesystem::path& { return resources_; }

  private:
    std::filesystem::path resources_;
};

TEST_F(ConfigTest, DirectoryPathReturnsCorrectPath) {
    auto dir = getDirectoryPath("problems");
    ASSERT_TRUE(dir.has_value());
    EXPECT_EQ(std::filesystem::weakly_canonical(std::filesystem::absolute(dir.value())),
              std::filesystem::weakly_canonical(getResourcesPath() / "problems"));
}

TEST_F(ConfigTest, DirectoryPathReturnsNulloptForMissingKey) {
    EXPECT_FALSE(getDirectoryPath("non_existent_key").has_value());
}

TEST_F(ConfigTest, MalformedJsonThrows) {
    Config::setConfigFilePath(getResourcesPath() / "bad_malformed.json");
    EXPECT_ANY_THROW(Config::reloadData());
}

TEST_F(ConfigTest, MissingConfigThrows) {
    Config::setConfigFilePath(getResourcesPath() / "non_existent.json");
    EXPECT_ANY_THROW(Config::reloadData());
}
```

## Building and Running Tests

```bash
# Configure (from project root)
cmake -B build

# Build all tests
cmake --build build

# Run all tests via CTest
cd build && ctest --output-on-failure

# Run a specific test target
cmake --build build --target oink_judge_config_tests
cd build && ctest -R oink_judge_config_tests --output-on-failure

# Run with verbose output
cd build && ctest -V
```
