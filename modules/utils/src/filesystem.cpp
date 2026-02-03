#include "oink_judge/utils/filesystem.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <sys/stat.h>
#include <zip.h>

namespace oink_judge::utils::filesystem {

auto loadFile(const fs::path& path) -> std::string {
    if (!fs::exists(path)) {
        throw std::runtime_error("File does not exist: " + path.string());
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string file_content(size, '\0');
    if (!file.read(file_content.data(), size)) {
        throw std::runtime_error("Failed to read file: " + path.string());
    }

    file.close();

    return file_content;
}

auto storeFile(const fs::path& path, const std::string& content) -> void {
    if (!createFileIfNotExists(path)) {
        throw std::runtime_error("Failed to create file: " + path.string());
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + path.string());
    }

    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!file) {
        throw std::runtime_error("Failed to write to file: " + path.string());
    }

    file.close();
}

auto createDirectoryIfNotExists(const fs::path& path) -> bool {
    if (!fs::exists(path)) {
        return fs::create_directories(path);
    }
    return true;
}

auto createFileIfNotExists(const fs::path& path) -> bool {
    if (!fs::exists(path)) {
        if (!createDirectoryIfNotExists(path.parent_path())) {
            return false;
        }

        std::ofstream file(path);
        if (!file) {
            return false;
        }
        file.close();
    }
    return true;
}

auto packDirectoryToZip(const fs::path& directory_path, const fs::path& zip_path) -> void {
    zip_t* zip = zip_open(zip_path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, nullptr);
    if (zip == nullptr) {
        throw std::runtime_error("Failed to create zip file: " + zip_path.string());
    }

    for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            fs::path relative_path = fs::relative(entry.path(), directory_path);

            zip_source_t* source = zip_source_file(zip, entry.path().c_str(), 0, 0);
            if (source == nullptr) {
                zip_close(zip);
                throw std::runtime_error("Failed to create zip source for file: " + entry.path().string());
            }

            zip_int64_t idx = zip_file_add(zip, relative_path.c_str(), source, ZIP_FL_ENC_UTF_8);
            if (idx < 0) {
                zip_source_free(source);
                zip_close(zip);
                throw std::runtime_error("Failed to add file to zip: " + relative_path.string());
            }

            struct stat st{};
            if (stat(entry.path().c_str(), &st) == 0) {
                zip_uint32_t attributes = (st.st_mode << 16); // NOLINT cppcoreguidelines-avoid-magic-numbers
                if (zip_file_set_external_attributes(zip, idx, 0, ZIP_OPSYS_UNIX, attributes) < 0) {
                    zip_close(zip);
                    throw std::runtime_error("Failed to set attributes for file: " + relative_path.string());
                }
            }
        }
    }

    if (zip_close(zip) < 0) {
        throw std::runtime_error("Failed to close zip file: " + zip_path.string());
    }
}

auto unpackZipToDirectory(const fs::path& zip_path, const fs::path& directory_path) -> void {
    if (!fs::exists(zip_path)) {
        throw std::runtime_error("Zip file does not exist: " + zip_path.string());
    }

    zip_t* zip = zip_open(zip_path.c_str(), ZIP_RDONLY, nullptr);
    if (zip == nullptr) {
        throw std::runtime_error("Failed to open zip file: " + zip_path.string());
    }

    fs::create_directories(directory_path);

    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(zip, i, 0);
        if (name == nullptr) {
            zip_close(zip);
            throw std::runtime_error("Failed to get name of entry");
        }

        fs::path file_path = directory_path / name;

        if (name[strlen(name) - 1] == '/') { // NOLINT cppcoreguidelines-pro-bounds-pointer-arithmetic
            fs::create_directories(file_path);
            continue;
        }

        zip_file_t* file = zip_fopen_index(zip, i, 0);
        if (file == nullptr) {
            zip_close(zip);
            throw std::runtime_error("Failed to open entry: " + std::string(name));
        }

        fs::create_directories(fs::path(file_path).parent_path());

        std::ofstream out_file(file_path, std::ios::binary);
        if (!out_file) {
            zip_fclose(file);
            zip_close(zip);
            throw std::runtime_error("Failed to open output file: " + file_path.string());
        }

        std::array<char, 8192> buffer{}; // NOLINT cppcoreguidelines-avoid-magic-numbers
        zip_int64_t bytes_read = 0;
        while ((bytes_read = zip_fread(file, buffer.data(), buffer.size())) > 0) {
            out_file.write(buffer.data(), bytes_read);
            if (!out_file) {
                zip_fclose(file);
                zip_close(zip);
                throw std::runtime_error("Failed to write to: " + file_path.string());
            }
        }
        out_file.close();
        zip_fclose(file);

        if (bytes_read < 0) {
            zip_close(zip);
            throw std::runtime_error("Read error in zip entry: " + std::string(name));
        }

        zip_uint8_t opsys = 0;
        zip_uint32_t attr = 0;
        if (zip_file_get_external_attributes(zip, i, 0, &opsys, &attr) == 0 && opsys == ZIP_OPSYS_UNIX) {
            mode_t mode = (attr >> 16) & 0xFFFF; // NOLINT cppcoreguidelines-avoid-magic-numbers
            if (mode != 0) {
                chmod(file_path.c_str(), mode);
            }
        }
    }

    zip_close(zip);
}

auto removeFileOrDirectory(const fs::path& path) -> void {
    if (fs::exists(path)) {
        fs::remove_all(path);
    }
}

auto clearDirectory(const fs::path& path) -> void {
    removeFileOrDirectory(path);
    fs::create_directories(path);
}

} // namespace oink_judge::utils::filesystem
