#include "oink_judge/utils/crypto.h"

#include <array>
#include <cstring>
#include <iomanip>
#include <sodium.h>
#include <sstream>

namespace oink_judge::utils::crypto {

auto sha256(const std::string& input) -> std::string {
    std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
    crypto_hash_sha256(digest.data(), reinterpret_cast<const unsigned char*>(input.data()), input.size()); // NOLINT

    std::ostringstream result;
    for (size_t byte_idx = 0; byte_idx < crypto_hash_sha256_BYTES; ++byte_idx) {
        result << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[byte_idx]); // NOLINT
    }
    return result.str();
}

auto toBase64(const std::string& input) -> std::string {
    size_t encoded_size = sodium_base64_encoded_len(input.size(), sodium_base64_VARIANT_ORIGINAL);
    std::string encoded(encoded_size, '\0');
    sodium_bin2base64(encoded.data(), encoded_size, reinterpret_cast<const unsigned char*>(input.data()), input.size(), // NOLINT
                      sodium_base64_VARIANT_ORIGINAL);
    // Remove the null terminator added by sodium_bin2base64
    encoded.resize(std::strlen(encoded.c_str()));
    return encoded;
}

auto fromBase64(const std::string& input) -> std::string {
    size_t decoded_max_size = input.size(); // Base64 expands data, so decoded size will be less than or equal to input size
    std::string decoded(decoded_max_size, '\0');
    size_t decoded_size = 0;
    if (sodium_base642bin(reinterpret_cast<unsigned char*>(decoded.data()), decoded_max_size, input.c_str(), // NOLINT
                          input.size(), nullptr, &decoded_size, nullptr, sodium_base64_VARIANT_ORIGINAL) != 0) {
        throw std::runtime_error("Invalid base64 input");
    }
    decoded.resize(decoded_size);
    return decoded;
}

} // namespace oink_judge::utils::crypto
