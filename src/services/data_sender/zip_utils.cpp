#include "services/data_sender/zip_utils.h"
#include <filesystem>
#include <fstream>
#include <zip.h>

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
            throw std::runtime_error("Failed to get name of entry in zip file: " + zip_path);
        }

        zip_file_t *file = zip_fopen_index(zip, i, 0);
        if (!file) {
            zip_close(zip);
            throw std::runtime_error("Failed to open entry in zip file: " + zip_path);
        }
        std::string file_path = unpack_path + "/" + name;
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
                throw std::runtime_error("Failed to write to output file: " + file_path);
            }
        }
        zip_fclose(file);
        out_file.close();
        if (bytes_read < 0) {
            zip_close(zip);
            throw std::runtime_error("Failed to read from zip entry: " + std::string(name));
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
            if (zip_file_add(zip, relative_path.c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
                zip_source_free(source);
                zip_close(zip);
                throw std::runtime_error("Failed to add file to zip: " + relative_path);
            }
        }
    }

    if (zip_close(zip) < 0) {
        throw std::runtime_error("Failed to close zip file: " + zip_path);
    }
}
    

} // namespace oink_judge::services::data_sender
