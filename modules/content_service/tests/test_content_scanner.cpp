#include "oink_judge/content_service/content_scanner.h"

#include <oink_judge/config/config.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>

using namespace oink_judge::content_service;
using nlohmann::json;
using oink_judge::config::Config;
namespace fs = std::filesystem;

class ContentScannerTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_content_scanner";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

    auto TearDown() -> void override {
        fs::remove_all(resources_ / "problems" / "empty");
        fs::remove_all(resources_ / "problems" / "nonexistent");
        fs::remove(resources_ / "problems" / "1" / "manifest.json");
        fs::remove(resources_ / "problems" / "1" / "temp_extra.txt");
        fs::remove_all(resources_ / "problems" / "1" / "sub");
    }

    auto getResourcesPath() -> const fs::path& { return resources_; }

    static auto writeTextFile(const fs::path& path, const std::string& content) -> void {
        std::ofstream f(path);
        f << content;
    }

    static auto readTextFile(const fs::path& path) -> std::string {
        std::ifstream f(path);
        return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
    }

  private:
    fs::path resources_;
};

// ---------------------------------------------------------------------------
// scanContent — happy path
// ---------------------------------------------------------------------------

TEST_F(ContentScannerTest, ScanContentContainsKnownFile) {
    ContentScanner scanner("problem", "1");
    json result = scanner.scanContent();
    ASSERT_TRUE(result.contains("files"));
    EXPECT_TRUE(result["files"].contains("input.txt"));
}

TEST_F(ContentScannerTest, ScanContentFileEntryContainsRequiredFields) {
    ContentScanner scanner("problem", "1");
    json result = scanner.scanContent();
    ASSERT_TRUE(result.contains("files"));
    ASSERT_TRUE(result["files"].contains("input.txt"));
    const auto& entry = result["files"]["input.txt"];
    EXPECT_TRUE(entry.contains("sha256"));
    EXPECT_TRUE(entry.contains("size"));
    EXPECT_TRUE(entry.contains("permissions"));
    EXPECT_TRUE(entry.contains("last_modified"));
}

TEST_F(ContentScannerTest, ScanContentSha256IsNonEmpty) {
    ContentScanner scanner("problem", "1");
    json result = scanner.scanContent();
    ASSERT_TRUE(result["files"].contains("input.txt"));
    EXPECT_FALSE(result["files"]["input.txt"]["sha256"].get<std::string>().empty());
}

TEST_F(ContentScannerTest, ScanContentFileSizeMatchesActual) {
    ContentScanner scanner("problem", "1");
    json result = scanner.scanContent();
    ASSERT_TRUE(result["files"].contains("input.txt"));
    auto actual_size = fs::file_size(getResourcesPath() / "problems" / "1" / "input.txt");
    EXPECT_EQ(result["files"]["input.txt"]["size"].get<uintmax_t>(), actual_size);
}

TEST_F(ContentScannerTest, ScanContentExcludesManifestJson) {
    fs::path manifest_path = getResourcesPath() / "problems" / "1" / "manifest.json";
    writeTextFile(manifest_path, "{}");

    ContentScanner scanner("problem", "1");
    json result = scanner.scanContent();

    if (result.contains("files")) {
        EXPECT_FALSE(result["files"].contains("manifest.json"));
    }
}

// ---------------------------------------------------------------------------
// scanContent — edge cases
// ---------------------------------------------------------------------------

TEST_F(ContentScannerTest, ScanContentOnEmptyDirectoryReturnsNoFiles) {
    fs::create_directories(getResourcesPath() / "problems" / "empty");
    ContentScanner scanner("problem", "empty");
    json result = scanner.scanContent();
    bool files_empty = !result.contains("files") || result["files"].empty();
    EXPECT_TRUE(files_empty);
}

TEST_F(ContentScannerTest, ScanContentCreatesDirectoryIfNotExists) {
    fs::path new_dir = getResourcesPath() / "problems" / "nonexistent";
    ASSERT_FALSE(fs::exists(new_dir));
    ContentScanner scanner("problem", "nonexistent");
    EXPECT_NO_THROW({ (void)scanner.scanContent(); });
    EXPECT_TRUE(fs::exists(new_dir));
}

// ---------------------------------------------------------------------------
// scanContentWithCache — sha256 reuse and recomputation
// ---------------------------------------------------------------------------

TEST_F(ContentScannerTest, ScanContentWithCacheReusesSha256ForUnchangedFile) {
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    ContentScanner scanner("problem", "1");
    json first = scanner.scanContent();
    ASSERT_TRUE(first["files"].contains("input.txt"));
    std::string first_sha256 = first["files"]["input.txt"]["sha256"].get<std::string>();

    json cached = scanner.scanContent();

    ASSERT_TRUE(cached["files"].contains("input.txt"));
    EXPECT_EQ(cached["files"]["input.txt"]["sha256"].get<std::string>(), first_sha256);
}

TEST_F(ContentScannerTest, ScanContentWithCacheRecomputesSha256WhenSizeChanges) {
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    ContentScanner scanner("problem", "1");
    json first = scanner.scanContent();
    ASSERT_TRUE(first["files"].contains("input.txt"));
    std::string first_sha256 = first["files"]["input.txt"]["sha256"].get<std::string>();

    fs::path input_path = getResourcesPath() / "problems" / "1" / "input.txt";
    std::string original = readTextFile(input_path);
    writeTextFile(input_path, original + " EXTRA_CONTENT_FOR_TEST");

    json cached = scanner.scanContent();
    writeTextFile(input_path, original); // restore before assertions

    ASSERT_TRUE(cached["files"].contains("input.txt"));
    EXPECT_NE(cached["files"]["input.txt"]["sha256"].get<std::string>(), first_sha256);
}

TEST_F(ContentScannerTest, CachedScanReusesPlantedSha256FromStoredManifest) {
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    ContentScanner scanner("problem", "1");
    json full = scanner.scanContent(); // first call is always a full scan
    ASSERT_TRUE(full["files"].contains("input.txt"));

    // Plant a stored manifest with a fake sha256 but correct size/last_modified.
    // A cached scan must echo the fake value back — proving it was read from
    // the cache rather than recomputed (recomputation would yield the real hash).
    json planted = full;
    planted["files"]["input.txt"]["sha256"] = "deadbeef";
    writeTextFile(getResourcesPath() / "problems" / "1" / "manifest.json", planted.dump(4));

    json cached = scanner.scanContent();

    ASSERT_TRUE(cached["files"].contains("input.txt"));
    EXPECT_EQ(cached["files"]["input.txt"]["sha256"].get<std::string>(), "deadbeef");
}

TEST_F(ContentScannerTest, CachedScanRecomputesSha256WhenLastModifiedDiffers) {
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    ContentScanner scanner("problem", "1");
    json full = scanner.scanContent();
    ASSERT_TRUE(full["files"].contains("input.txt"));
    std::string real_sha256 = full["files"]["input.txt"]["sha256"].get<std::string>();

    // Same size, stale last_modified — the cache entry must be rejected.
    json planted = full;
    planted["files"]["input.txt"]["sha256"] = "deadbeef";
    planted["files"]["input.txt"]["last_modified"] = full["files"]["input.txt"]["last_modified"].get<int64_t>() - 1;
    writeTextFile(getResourcesPath() / "problems" / "1" / "manifest.json", planted.dump(4));

    json cached = scanner.scanContent();

    ASSERT_TRUE(cached["files"].contains("input.txt"));
    EXPECT_EQ(cached["files"]["input.txt"]["sha256"].get<std::string>(), real_sha256);
}

TEST_F(ContentScannerTest, CachedScanToleratesMalformedStoredManifest) {
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    ContentScanner scanner("problem", "1");
    (void)scanner.scanContent(); // full scan

    writeTextFile(getResourcesPath() / "problems" / "1" / "manifest.json", "not json {{{");

    json cached;
    ASSERT_NO_THROW(cached = scanner.scanContent());
    ASSERT_TRUE(cached["files"].contains("input.txt"));
    EXPECT_FALSE(cached["files"]["input.txt"]["sha256"].get<std::string>().empty());
}

TEST_F(ContentScannerTest, CachedScanDropsFilesRemovedFromDisk) {
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    fs::path extra_path = getResourcesPath() / "problems" / "1" / "temp_extra.txt";
    writeTextFile(extra_path, "temporary file");

    ContentScanner scanner("problem", "1");
    json full = scanner.scanContent();
    ASSERT_TRUE(full["files"].contains("temp_extra.txt"));

    fs::remove(extra_path);

    json cached = scanner.scanContent();
    ASSERT_TRUE(cached.contains("files"));
    EXPECT_FALSE(cached["files"].contains("temp_extra.txt"));
}

TEST_F(ContentScannerTest, ZeroRescanIntervalAlwaysFullScansIgnoringStoredCache) {
    // good_config.json has full_rescan_interval = 0.0 — every scan must be full.
    ContentScanner scanner("problem", "1");
    json full = scanner.scanContent();
    ASSERT_TRUE(full["files"].contains("input.txt"));
    std::string real_sha256 = full["files"]["input.txt"]["sha256"].get<std::string>();

    // Plant a matching cache entry with a fake sha256; a full scan must ignore it.
    json planted = full;
    planted["files"]["input.txt"]["sha256"] = "deadbeef";
    writeTextFile(getResourcesPath() / "problems" / "1" / "manifest.json", planted.dump(4));

    json second = scanner.scanContent();

    ASSERT_TRUE(second["files"].contains("input.txt"));
    EXPECT_EQ(second["files"]["input.txt"]["sha256"].get<std::string>(), real_sha256);
}

// ---------------------------------------------------------------------------
// scanContent — nested directories
// ---------------------------------------------------------------------------

TEST_F(ContentScannerTest, ScanContentListsNestedFilesWithRelativePaths) {
    fs::create_directories(getResourcesPath() / "problems" / "1" / "sub" / "dir");
    writeTextFile(getResourcesPath() / "problems" / "1" / "sub" / "dir" / "file.txt", "nested content");

    ContentScanner scanner("problem", "1");
    json result = scanner.scanContent();

    ASSERT_TRUE(result.contains("files"));
    ASSERT_TRUE(result["files"].contains("sub/dir/file.txt"));
    EXPECT_FALSE(result["files"].contains("sub"));
    EXPECT_FALSE(result["files"].contains("sub/dir"));
    EXPECT_FALSE(result["files"]["sub/dir/file.txt"]["sha256"].get<std::string>().empty());
}

// ---------------------------------------------------------------------------
// Config error paths
// ---------------------------------------------------------------------------

TEST_F(ContentScannerTest, ConstructorThrowsWhenRescanIntervalMissingFromConfig) {
    Config::setConfigFilePath(getResourcesPath() / "config_no_timing.json");
    Config::reloadData();

    EXPECT_THROW(ContentScanner("problem", "1"), std::runtime_error);
}

TEST_F(ContentScannerTest, ScanContentThrowsForUnknownContentType) {
    ContentScanner scanner("submission", "1");
    EXPECT_THROW((void)scanner.scanContent(), std::runtime_error);
}
