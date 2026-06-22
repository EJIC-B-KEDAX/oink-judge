#include "oink_judge/utils/filesystem.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

using namespace oink_judge::utils::filesystem;
namespace fs = std::filesystem;

class FilesystemTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        root_ = fs::temp_directory_path() / "oink_judge_utils_filesystem_tests";
        removeFileOrDirectory(root_);
        createDirectoryIfNotExists(root_);
    }

    auto TearDown() -> void override { removeFileOrDirectory(root_); }

    auto root() const -> const fs::path& { return root_; }

  private:
    fs::path root_;
};

// ---------------------------------------------------------------------------
// storeFile / loadFile
// ---------------------------------------------------------------------------

TEST_F(FilesystemTest, StoreAndLoadFileRoundTrip) {
    const fs::path file_path = root() / "nested" / "data.bin";
    const std::string content = "line one\nline two\x00\xff"; // NOLINT

    storeFile(file_path, content);

    EXPECT_EQ(loadFile(file_path), content);
}

TEST_F(FilesystemTest, LoadNonExistentFileThrows) { EXPECT_THROW(loadFile(root() / "missing.txt"), std::runtime_error); }

// ---------------------------------------------------------------------------
// createDirectoryIfNotExists / createFileIfNotExists
// ---------------------------------------------------------------------------

TEST_F(FilesystemTest, CreateDirectoryIfNotExistsCreatesNestedPath) {
    const fs::path directory_path = root() / "a" / "b" / "c";

    EXPECT_TRUE(createDirectoryIfNotExists(directory_path));
    EXPECT_TRUE(fs::is_directory(directory_path));
}

TEST_F(FilesystemTest, CreateDirectoryIfNotExistsReturnsTrueForExistingDirectory) {
    const fs::path directory_path = root() / "existing";

    ASSERT_TRUE(createDirectoryIfNotExists(directory_path));
    EXPECT_TRUE(createDirectoryIfNotExists(directory_path));
}

TEST_F(FilesystemTest, CreateFileIfNotExistsCreatesParentDirectories) {
    const fs::path file_path = root() / "parents" / "child.txt";

    EXPECT_TRUE(createFileIfNotExists(file_path));
    EXPECT_TRUE(fs::is_regular_file(file_path));
}

// ---------------------------------------------------------------------------
// packDirectoryToZip / unpackZipToDirectory
// ---------------------------------------------------------------------------

TEST_F(FilesystemTest, PackAndUnpackDirectoryRoundTrip) {
    const fs::path source_dir = root() / "source";
    const fs::path nested_dir = source_dir / "nested";
    const fs::path zip_path = root() / "archive.zip";
    const fs::path unpacked_dir = root() / "unpacked";

    createDirectoryIfNotExists(nested_dir);
    storeFile(source_dir / "top.txt", "top-level");
    storeFile(nested_dir / "inner.txt", "nested-content");

    packDirectoryToZip(source_dir, zip_path);
    unpackZipToDirectory(zip_path, unpacked_dir);

    EXPECT_EQ(loadFile(unpacked_dir / "top.txt"), "top-level");
    EXPECT_EQ(loadFile(unpacked_dir / "nested" / "inner.txt"), "nested-content");
}

TEST_F(FilesystemTest, UnpackNonExistentZipThrows) {
    EXPECT_THROW(unpackZipToDirectory(root() / "missing.zip", root() / "out"), std::runtime_error);
}

// ---------------------------------------------------------------------------
// removeFileOrDirectory / clearDirectory
// ---------------------------------------------------------------------------

TEST_F(FilesystemTest, RemoveFileOrDirectoryDeletesNestedTree) {
    const fs::path target = root() / "to_remove";
    storeFile(target / "file.txt", "delete me");

    removeFileOrDirectory(target);

    EXPECT_FALSE(fs::exists(target));
}

TEST_F(FilesystemTest, ClearDirectoryRemovesContentsButKeepsDirectory) {
    const fs::path directory_path = root() / "clear_me";
    storeFile(directory_path / "one.txt", "1");
    storeFile(directory_path / "two.txt", "2");

    clearDirectory(directory_path);

    EXPECT_TRUE(fs::is_directory(directory_path));
    EXPECT_TRUE(fs::is_empty(directory_path));
}

// ---------------------------------------------------------------------------
// setPermissions / getPermissions
// ---------------------------------------------------------------------------

TEST_F(FilesystemTest, SetAndGetPermissionsRoundTrip) {
    const fs::path file_path = root() / "permissions.txt";
    storeFile(file_path, "permissions");

    const fs::perms desired_permissions = fs::perms::owner_read | fs::perms::owner_write;

    setPermissions(file_path, desired_permissions);

    EXPECT_EQ(getPermissions(file_path), desired_permissions);
}

TEST_F(FilesystemTest, GetPermissionsThrowsForMissingPath) {
    EXPECT_THROW(getPermissions(root() / "missing.txt"), std::runtime_error);
}
