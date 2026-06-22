#include "oink_judge/content_service/content_manifest.h"

#include <gtest/gtest.h>

#include <filesystem>

using namespace oink_judge::content_service;
using nlohmann::json;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// compareManifests(json, json)
// ---------------------------------------------------------------------------

TEST(CompareManifestsTest, BothEmptyReturnsNoChanges) {
    auto changes = compareManifests(json{}, json{});
    EXPECT_TRUE(changes.empty());
}

TEST(CompareManifestsTest, FileAddedInNewManifest) {
    json old_manifest = {};
    json new_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::ADDED);
    EXPECT_EQ(changes[0].file_path, fs::path("input.txt"));
}

TEST(CompareManifestsTest, FileRemovedFromNewManifest) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");
    json new_manifest = {};

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::REMOVED);
    EXPECT_EQ(changes[0].file_path, fs::path("input.txt"));
}

TEST(CompareManifestsTest, FileSha256ChangedReturnsModified) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"xyz","permissions":420}}})");

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::MODIFIED);
    EXPECT_EQ(changes[0].file_path, fs::path("input.txt"));
}

TEST(CompareManifestsTest, PermissionsChangedReturnsAttributesChanged) {
    json old_manifest = json::parse(R"({"files":{"checker":{"sha256":"abc","permissions":420}}})");
    json new_manifest = json::parse(R"({"files":{"checker":{"sha256":"abc","permissions":493}}})");

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::ATTRIBUTES_CHANGED);
    EXPECT_EQ(changes[0].file_path, fs::path("checker"));
}

TEST(CompareManifestsTest, IdenticalManifestsReturnsNoChanges) {
    json manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");

    auto changes = compareManifests(manifest, manifest);

    EXPECT_TRUE(changes.empty());
}

TEST(CompareManifestsTest, MultipleChangesDetected) {
    json old_manifest = json::parse(R"({
        "files":{
            "input.txt":{"sha256":"abc","permissions":420},
            "old_file.txt":{"sha256":"def","permissions":420}
        }
    })");
    json new_manifest = json::parse(R"({
        "files":{
            "input.txt":{"sha256":"xyz","permissions":420},
            "new_file.txt":{"sha256":"ghi","permissions":420}
        }
    })");

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 3U);
    bool has_modified = false;
    bool has_added = false;
    bool has_removed = false;
    for (const auto& change : changes) {
        if (change.type == ContentChange::Type::MODIFIED) {
            has_modified = true;
        }
        if (change.type == ContentChange::Type::ADDED) {
            has_added = true;
        }
        if (change.type == ContentChange::Type::REMOVED) {
            has_removed = true;
        }
    }
    EXPECT_TRUE(has_modified);
    EXPECT_TRUE(has_added);
    EXPECT_TRUE(has_removed);
}

TEST(CompareManifestsTest, OldManifestHasNoFilesKeyTreatedAsEmpty) {
    json old_manifest = json::parse(R"({"other_key":"value"})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::ADDED);
}

TEST(CompareManifestsTest, NewManifestHasNoFilesKeyTreatedAsEmpty) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");
    json new_manifest = json::parse(R"({"other_key":"value"})");

    auto changes = compareManifests(old_manifest, new_manifest);

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::REMOVED);
}

// ---------------------------------------------------------------------------
// compareManifests — malformed file entries (missing keys must not crash)
// ---------------------------------------------------------------------------

TEST(CompareManifestsTest, EntryMissingSha256InNewManifestTreatedAsModified) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"permissions":420}}})");

    std::vector<ContentChange> changes;
    ASSERT_NO_THROW(changes = compareManifests(old_manifest, new_manifest));

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::MODIFIED);
    EXPECT_EQ(changes[0].file_path, fs::path("input.txt"));
}

TEST(CompareManifestsTest, EntryMissingSha256InOldManifestTreatedAsModified) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"permissions":420}}})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");

    std::vector<ContentChange> changes;
    ASSERT_NO_THROW(changes = compareManifests(old_manifest, new_manifest));

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::MODIFIED);
}

TEST(CompareManifestsTest, BothEntriesMissingSha256ReturnsNoModification) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"permissions":420}}})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"permissions":420}}})");

    std::vector<ContentChange> changes;
    ASSERT_NO_THROW(changes = compareManifests(old_manifest, new_manifest));

    EXPECT_TRUE(changes.empty());
}

TEST(CompareManifestsTest, EntryMissingPermissionsTreatedAsAttributesChanged) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","permissions":420}}})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc"}}})");

    std::vector<ContentChange> changes;
    ASSERT_NO_THROW(changes = compareManifests(old_manifest, new_manifest));

    ASSERT_EQ(changes.size(), 1U);
    EXPECT_EQ(changes[0].type, ContentChange::Type::ATTRIBUTES_CHANGED);
}

TEST(CompareManifestsTest, BothEntriesMissingPermissionsReturnsNoChanges) {
    json old_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc"}}})");
    json new_manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc"}}})");

    std::vector<ContentChange> changes;
    ASSERT_NO_THROW(changes = compareManifests(old_manifest, new_manifest));

    EXPECT_TRUE(changes.empty());
}

// ---------------------------------------------------------------------------
// getPermissionsFromManifest
// ---------------------------------------------------------------------------

TEST(GetPermissionsFromManifestTest, FilePresentReturnsCorrectPermissions) {
    // 0644 octal = 420 decimal
    json manifest = json::parse(R"({"files":{"input.txt":{"permissions":420}}})");
    auto perms = getPermissionsFromManifest(manifest, "input.txt");
    EXPECT_EQ(perms, static_cast<fs::perms>(420));
}

TEST(GetPermissionsFromManifestTest, ExecutablePermissionsRoundTrip) {
    // 0755 octal = 493 decimal
    json manifest = json::parse(R"({"files":{"checker":{"permissions":493}}})");
    auto perms = getPermissionsFromManifest(manifest, "checker");
    EXPECT_EQ(perms, static_cast<fs::perms>(493));
}

TEST(GetPermissionsFromManifestTest, FileAbsentReturnsUnknown) {
    json manifest = json::parse(R"({"files":{"input.txt":{"permissions":420}}})");
    auto perms = getPermissionsFromManifest(manifest, "nonexistent.txt");
    EXPECT_EQ(perms, fs::perms::unknown);
}

TEST(GetPermissionsFromManifestTest, EmptyManifestReturnsUnknown) {
    json manifest = {};
    auto perms = getPermissionsFromManifest(manifest, "input.txt");
    EXPECT_EQ(perms, fs::perms::unknown);
}

TEST(GetPermissionsFromManifestTest, FileEntryMissingPermissionsKeyReturnsUnknown) {
    json manifest = json::parse(R"({"files":{"input.txt":{"sha256":"abc","size":12}}})");
    auto perms = getPermissionsFromManifest(manifest, "input.txt");
    EXPECT_EQ(perms, fs::perms::unknown);
}
