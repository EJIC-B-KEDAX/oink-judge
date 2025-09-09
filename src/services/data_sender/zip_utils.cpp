#include "services/data_sender/zip_utils.h"
#include <filesystem>
#include <fstream>
#include <zip.h>
#include <stdexcept>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

namespace oink_judge::services::data_sender {

std::string get_zip(const std::string &zip_path) {
    if (!std::filesystem::exists(zip_path)) {
        throw std::runtime_error("Zip file does not exist: " + zip_path);
    }

    std::ifstream zip_file(zip_path, std::ios::binary | std::ios::ate);
    if (!zip_file) {
        throw std::runtime_error("Failed to open zip file: " + zip_path);
    }

    std::streamsize size = zip_file.tellg();
    zip_file.seekg(0, std::ios::beg);
    std::string zip_content(size, '\0');
    if (!zip_file.read(zip_content.data(), size)) {
        throw std::runtime_error("Failed to read zip file: " + zip_path);
    }

    zip_file.close();

    return zip_content;
}

void store_zip(const std::string &zip_path, const std::string &zip_content) {
    std::ofstream zip_file(zip_path, std::ios::binary);
    if (!zip_file) {
        throw std::runtime_error("Failed to open zip file for writing: " + zip_path);
    }

    zip_file.write(zip_content.data(), zip_content.size());
    if (!zip_file) {
        throw std::runtime_error("Failed to write to zip file: " + zip_path);
    }

    zip_file.close();
}

void unpack_zip(const std::string &zip_path, const std::string &unpack_path) {
    if (!std::filesystem::exists(zip_path)) {
        throw std::runtime_error("Zip file does not exist: " + zip_path);
    }

    zip_t *zip = zip_open(zip_path.c_str(), ZIP_RDONLY, nullptr);
    if (!zip) {
        throw std::runtime_error("Failed to open zip file: " + zip_path);
    }

    std::filesystem::create_directories(unpack_path);

    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char *name = zip_get_name(zip, i, 0);
        if (!name) {
            zip_close(zip);
            throw std::runtime_error("Failed to get name of entry");
        }

        std::string file_path = unpack_path + "/" + name;

        if (name[strlen(name) - 1] == '/') {
            std::filesystem::create_directories(file_path);
            continue;
        }

        zip_file_t *file = zip_fopen_index(zip, i, 0);
        if (!file) {
            zip_close(zip);
            throw std::runtime_error("Failed to open entry: " + std::string(name));
        }

        std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());

        std::ofstream out_file(file_path, std::ios::binary);
        if (!out_file) {
            zip_fclose(file);
            zip_close(zip);
            throw std::runtime_error("Failed to open output file: " + file_path);
        }

        char buffer[8192];
        zip_int64_t bytes_read;
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            out_file.write(buffer, bytes_read);
            if (!out_file) {
                zip_fclose(file);
                zip_close(zip);
                throw std::runtime_error("Failed to write to: " + file_path);
            }
        }
        out_file.close();
        zip_fclose(file);

        if (bytes_read < 0) {
            zip_close(zip);
            throw std::runtime_error("Read error in zip entry: " + std::string(name));
        }

        zip_uint8_t opsys;
        zip_uint32_t attr;
        if (zip_file_get_external_attributes(zip, i, 0, &opsys, &attr) == 0) {
            if (opsys == ZIP_OPSYS_UNIX) {
                mode_t mode = (attr >> 16) & 0xFFFF;
                if (mode != 0) {
                    chmod(file_path.c_str(), mode);
                }
            }
        }
    }

    zip_close(zip);
}

void pack_zip(const std::string &zip_path, const std::string &source_path) {
    zip_t *zip = zip_open(zip_path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, nullptr);
    if (!zip) {
        throw std::runtime_error("Failed to create zip file: " + zip_path);
    }

    for (const auto &entry : std::filesystem::recursive_directory_iterator(source_path)) {
        if (entry.is_regular_file()) {
            std::string relative_path = std::filesystem::relative(entry.path(), source_path).string();

            zip_source_t *source = zip_source_file(zip, entry.path().c_str(), 0, 0);
            if (!source) {
                zip_close(zip);
                throw std::runtime_error("Failed to create zip source for file: " + entry.path().string());
            }

            zip_int64_t idx = zip_file_add(zip, relative_path.c_str(), source, ZIP_FL_ENC_UTF_8);
            if (idx < 0) {
                zip_source_free(source);
                zip_close(zip);
                throw std::runtime_error("Failed to add file to zip: " + relative_path);
            }

            struct stat st;
            if (stat(entry.path().c_str(), &st) == 0) {
                zip_uint32_t attributes = (st.st_mode << 16);
                if (zip_file_set_external_attributes(zip, idx, 0, ZIP_OPSYS_UNIX, attributes) < 0) {
                    zip_close(zip);
                    throw std::runtime_error("Failed to set attributes for file: " + relative_path);
                }
            }
        }
    }

    if (zip_close(zip) < 0) {
        throw std::runtime_error("Failed to close zip file: " + zip_path);
    }
}

} // namespace oink_judge::services::data_sender
