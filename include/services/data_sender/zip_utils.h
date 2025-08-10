#pragma once

#include <string>

namespace oink_judge::services::data_sender {

std::string get_zip(const std::string &zip_path);

void store_zip(const std::string &zip_path, const std::string &zip_content);

void unpack_zip(const std::string &zip_path, const std::string &unpack_path);

void pack_zip(const std::string &zip_path, const std::string &source_path);

} // namespace oink_judge::services::data_sender
