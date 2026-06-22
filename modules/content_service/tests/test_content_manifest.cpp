#include "oink_judge/content_service/content_manifest.h"

#include <oink_judge/config/config.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

using namespace oink_judge::content_service;
using nlohmann::json;
using oink_judge::config::Config;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Fixture — loads config with full_rescan_interval=0.0 so every toJson()
// call triggers a full filesystem scan (avoids the storedManifestToJson IO
// issue on the first call with a fast-rescan interval).
// ---------------------------------------------------------------------------

class ContentManifestTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_content_manifest";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

    auto TearDown() -> void override {
        fs::remove(resources_ / "problems" / "1" / "manifest.json");
        // Restore the full-rescan config in case a test switched it
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::reloadData();
    }

    auto getResourcesPath() -> const fs::path& { return resources_; }

  private:
    fs::path resources_;
};

// ---------------------------------------------------------------------------
// Constructor / getters
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, ConstructorStoresContentType) {
    ContentManifest manifest("problem", "1");
    EXPECT_EQ(manifest.getContentType(), "problem");
}

TEST_F(ContentManifestTest, ConstructorStoresContentId) {
    ContentManifest manifest("problem", "1");
    EXPECT_EQ(manifest.getContentId(), "1");
}

// ---------------------------------------------------------------------------
// getPathToManifestFile
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, GetPathToManifestFileReturnsExpectedPath) {
    ContentManifest manifest("problem", "1");
    auto actual = fs::weakly_canonical(fs::absolute(manifest.getPathToManifestFile()));
    auto expected = fs::weakly_canonical(fs::absolute(getResourcesPath() / "problems" / "1" / "manifest.json"));
    EXPECT_EQ(actual, expected);
}

// ---------------------------------------------------------------------------
// toJson — structure
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, ToJsonContainsFilesKey) {
    ContentManifest manifest("problem", "1");
    auto result = manifest.toJson();
    EXPECT_TRUE(result.contains("files"));
}

TEST_F(ContentManifestTest, ToJsonContainsKnownFile) {
    ContentManifest manifest("problem", "1");
    auto result = manifest.toJson();
    ASSERT_TRUE(result.contains("files"));
    ASSERT_TRUE(result["files"].contains("input.txt"));
}

TEST_F(ContentManifestTest, ToJsonFileEntryContainsRequiredFields) {
    ContentManifest manifest("problem", "1");
    auto result = manifest.toJson();
    ASSERT_TRUE(result.contains("files"));
    ASSERT_TRUE(result["files"].contains("input.txt"));

    const auto& entry = result["files"]["input.txt"];
    EXPECT_TRUE(entry.contains("sha256"));
    EXPECT_TRUE(entry.contains("size"));
    EXPECT_TRUE(entry.contains("permissions"));
    EXPECT_TRUE(entry.contains("last_modified"));
}

TEST_F(ContentManifestTest, ToJsonSha256IsNonEmpty) {
    ContentManifest manifest("problem", "1");
    auto result = manifest.toJson();
    ASSERT_TRUE(result["files"].contains("input.txt"));
    EXPECT_FALSE(result["files"]["input.txt"]["sha256"].get<std::string>().empty());
}

TEST_F(ContentManifestTest, ToJsonExcludesManifestJsonItself) {
    ContentManifest manifest("problem", "1");
    auto result = manifest.toJson();
    if (result.contains("files")) {
        EXPECT_FALSE(result["files"].contains("manifest.json"));
    }
}

TEST_F(ContentManifestTest, ToJsonFileSizeMatchesActualFile) {
    ContentManifest manifest("problem", "1");
    auto result = manifest.toJson();
    ASSERT_TRUE(result["files"].contains("input.txt"));

    auto actual_size = fs::file_size(getResourcesPath() / "problems" / "1" / "input.txt");
    EXPECT_EQ(result["files"]["input.txt"]["size"].get<uintmax_t>(), actual_size);
}

// ---------------------------------------------------------------------------
// toJson — empty directory
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, ToJsonOnEmptyDirectoryHasEmptyOrMissingFilesKey) {
    ContentManifest manifest("problem", "empty");
    fs::create_directories(getResourcesPath() / "problems" / "empty");

    auto result = manifest.toJson();

    // Either no "files" key, or "files" is an empty object
    bool files_empty = !result.contains("files") || result["files"].empty();
    EXPECT_TRUE(files_empty);

    fs::remove_all(getResourcesPath() / "problems" / "empty");
}

TEST_F(ContentManifestTest, ToJsonWhenDirectoryDoesNotExistCreatesDirectoryAndReturnsEmpty) {
    fs::path new_dir = getResourcesPath() / "problems" / "nonexistent_id";
    ASSERT_FALSE(fs::exists(new_dir));

    ContentManifest manifest("problem", "nonexistent_id");
    json result = manifest.toJson();

    EXPECT_TRUE(fs::exists(new_dir));
    bool files_empty = !result.contains("files") || result["files"].empty();
    EXPECT_TRUE(files_empty);

    fs::remove_all(new_dir);
}

// ---------------------------------------------------------------------------
// updateManifest — persistence
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, UpdateManifestWritesFileConsistentWithToJson) {
    ContentManifest manifest("problem", "1");
    manifest.updateManifest();

    fs::path manifest_path = manifest.getPathToManifestFile();
    ASSERT_TRUE(fs::exists(manifest_path));

    std::ifstream file(manifest_path);
    json stored = json::parse(std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()));

    EXPECT_EQ(stored, manifest.toJson());
}

// ---------------------------------------------------------------------------
// Config error paths
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, ToJsonThrowsForUnknownContentType) {
    ContentManifest manifest("unknown_content_type", "1");
    EXPECT_THROW((void)manifest.toJson(), std::runtime_error);
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, ToStringReturnsValidJsonString) {
    ContentManifest manifest("problem", "1");
    std::string result = manifest.toString();
    json parsed;
    EXPECT_NO_THROW(parsed = json::parse(result));
}

TEST_F(ContentManifestTest, ToStringAndToJsonAreConsistent) {
    ContentManifest manifest("problem", "1");
    json from_json = manifest.toJson();
    json from_string = json::parse(manifest.toString());
    EXPECT_EQ(from_json, from_string);
}

// ---------------------------------------------------------------------------
// getManifestSignature
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, GetManifestSignatureIsDeterministic) {
    auto sig1 = getManifestSignature("problem", "1");
    auto sig2 = getManifestSignature("problem", "1");
    EXPECT_EQ(sig1, sig2);
}

TEST_F(ContentManifestTest, GetManifestSignatureDiffersForDifferentContentIds) {
    auto sig1 = getManifestSignature("problem", "1");
    auto sig2 = getManifestSignature("problem", "2");
    EXPECT_NE(sig1, sig2);
}

TEST_F(ContentManifestTest, GetManifestSignatureDiffersForDifferentContentTypes) {
    auto sig1 = getManifestSignature("problem", "1");
    auto sig2 = getManifestSignature("submission", "1");
    EXPECT_NE(sig1, sig2);
}

// ---------------------------------------------------------------------------
// compareManifests(ContentManifest&, json&) overload
// ---------------------------------------------------------------------------

TEST_F(ContentManifestTest, CompareManifestsManifestOverloadReturnsNoChangesForSelf) {
    ContentManifest manifest("problem", "1");
    json manifest_json = manifest.toJson();

    auto changes = compareManifests(manifest, manifest_json);

    EXPECT_TRUE(changes.empty());
}

TEST_F(ContentManifestTest, CompareManifestsManifestOverloadDetectsAddedFile) {
    ContentManifest manifest("problem", "1");
    json manifest_json = manifest.toJson();

    // Add a file to the server manifest that doesn't exist locally
    manifest_json["files"]["new_file.txt"] = {{"sha256", "abc"}, {"permissions", 420}}; // NOLINT

    auto changes = compareManifests(manifest, manifest_json);

    bool has_added = false;
    for (const auto& change : changes) {
        if (change.type == ContentChange::Type::ADDED && change.file_path == fs::path("new_file.txt")) {
            has_added = true;
        }
    }
    EXPECT_TRUE(has_added);
}

// ---------------------------------------------------------------------------
// Fast rescan — sha256 reuse for unchanged files
// ---------------------------------------------------------------------------

class ContentManifestFastRescanTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_content_manifest";
        // First load the full-rescan config to seed the manifest file
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
        // Seed the manifest.json by doing one full scan
        ContentManifest seed("problem", "1");
        seed.updateManifest();
    }

    auto TearDown() -> void override {
        fs::remove(resources_ / "problems" / "1" / "manifest.json");
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::reloadData();
    }

    auto getResourcesPath() -> const fs::path& { return resources_; }

  private:
    fs::path resources_;
};

namespace {
auto readTextFile(const fs::path& path) -> std::string {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

auto writeTextFile(const fs::path& path, const std::string& content) -> void {
    std::ofstream f(path);
    f << content;
}
} // namespace

TEST_F(ContentManifestFastRescanTest, FastRescanPreservesExistingSha256ForUnchangedFile) {
    // Switch to large-interval config so the next ContentManifest uses fast rescan
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    // Capture the sha256 written by the full scan seed
    json seeded = json::parse(readTextFile(getResourcesPath() / "problems" / "1" / "manifest.json"));
    ASSERT_TRUE(seeded["files"].contains("input.txt"));
    std::string seeded_sha256 = seeded["files"]["input.txt"]["sha256"].get<std::string>();

    // Construct a new ContentManifest — last_full_rescan_ = now, interval = 3600s → fast rescan
    ContentManifest manifest("problem", "1");
    json result = manifest.toJson();

    ASSERT_TRUE(result["files"].contains("input.txt"));
    EXPECT_EQ(result["files"]["input.txt"]["sha256"].get<std::string>(), seeded_sha256);
}

TEST_F(ContentManifestFastRescanTest, FastRescanRecomputesSha256WhenFileSizeChanges) {
    // Capture the sha256 from the full scan seed
    json seeded = json::parse(readTextFile(getResourcesPath() / "problems" / "1" / "manifest.json"));
    std::string original_sha256 = seeded["files"]["input.txt"]["sha256"].get<std::string>();

    // Switch to fast-rescan config
    Config::setConfigFilePath(getResourcesPath() / "good_config_fast_rescan.json");
    Config::reloadData();

    // Overwrite input.txt with different (longer) content to change its size
    fs::path input_path = getResourcesPath() / "problems" / "1" / "input.txt";
    std::string original_content = readTextFile(input_path);
    writeTextFile(input_path, original_content + " MODIFIED_CONTENT_FOR_TEST");

    ContentManifest manifest("problem", "1");
    json result = manifest.toJson();

    // Restore the original file content before asserting (TearDown also cleans up manifest.json)
    writeTextFile(input_path, original_content);

    ASSERT_TRUE(result["files"].contains("input.txt"));
    EXPECT_NE(result["files"]["input.txt"]["sha256"].get<std::string>(), original_sha256);
}
