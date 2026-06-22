#include "oink_judge/content_service/manifest_storage.h"

#include <oink_judge/config/config.h>

#include <gtest/gtest.h>

#include <filesystem>

using namespace oink_judge::content_service;
using oink_judge::config::Config;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

class ManifestStorageTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_manifest_storage";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

    auto TearDown() -> void override {
        // Clean up any manifest.json files written during tests
        fs::remove(resources_ / "problems" / "1" / "manifest.json");
        fs::remove(resources_ / "problems" / "2" / "manifest.json");
    }

    auto getResourcesPath() -> const fs::path& { return resources_; }

  private:
    fs::path resources_;
};

// ---------------------------------------------------------------------------
// Identity / singleton behaviour
// ---------------------------------------------------------------------------

TEST_F(ManifestStorageTest, GetManifestReturnsSameObjectForSameKey) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m1 = storage.getManifest("problem", "1");
    const ContentManifest& m2 = storage.getManifest("problem", "1");
    EXPECT_EQ(&m1, &m2);
}

TEST_F(ManifestStorageTest, GetManifestReturnsDifferentObjectsForDifferentContentIds) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m1 = storage.getManifest("problem", "1");
    const ContentManifest& m2 = storage.getManifest("problem", "2");
    EXPECT_NE(&m1, &m2);
}

TEST_F(ManifestStorageTest, GetManifestReturnsDifferentObjectsForDifferentContentTypes) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m1 = storage.getManifest("problem", "1");
    const ContentManifest& m2 = storage.getManifest("submission", "1");
    EXPECT_NE(&m1, &m2);
}

// ---------------------------------------------------------------------------
// Correct construction of the stored ContentManifest
// ---------------------------------------------------------------------------

TEST_F(ManifestStorageTest, GetManifestReturnsCorrectContentType) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m = storage.getManifest("problem", "1");
    EXPECT_EQ(m.getContentType(), "problem");
}

TEST_F(ManifestStorageTest, GetManifestReturnsCorrectContentId) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m = storage.getManifest("problem", "1");
    EXPECT_EQ(m.getContentId(), "1");
}

TEST_F(ManifestStorageTest, GetManifestReturnsCorrectContentIdForSecondEntry) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m = storage.getManifest("problem", "2");
    EXPECT_EQ(m.getContentId(), "2");
}

// ---------------------------------------------------------------------------
// Lazy initialization — stored manifest can produce a valid JSON
// ---------------------------------------------------------------------------

TEST_F(ManifestStorageTest, GetManifestCanProduceValidJson) {
    auto& storage = ManifestStorage::instance();
    const ContentManifest& m = storage.getManifest("problem", "1");
    EXPECT_NO_THROW({
        auto result = m.toJson();
        EXPECT_TRUE(result.contains("files"));
    });
}
