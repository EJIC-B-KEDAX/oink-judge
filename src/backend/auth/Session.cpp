#include "backend/auth/Session.h"
#include "config/Config.h"
#include <ctime>
#include <random>
#include <sodium.h>

namespace oink_judge::backend::auth {

namespace {

std::random_device rd;
std::mt19937_64 rng(rd());

} // namespace

using Config = config::Config;

Session::Session(const std::string &username) : _username(username) {
    _session_id = "";
    _expire_at = 0;
}

void Session::generate_session() {
    if (!_session_id.empty()) {
        return;
    }

    _session_id = generate_session_id();
    _expire_at = generate_expired_at();
}

bool Session::is_valid() const {
    time_t current_time = std::time(nullptr);
    return current_time <= _expire_at;
}

std::string Session::get_session_id() const {
    if (!is_valid()) {
        return "";
    }

    return _session_id;
}

const std::string& Session::get_username() const {
    return _username;
}

time_t Session::get_expire_at() const {
    return _expire_at;
}

std::string Session::generate_session_id() {
    time_t current_time = std::time(nullptr);
    std::string data = std::to_string(current_time) + "." + std::to_string(rng()) + "." + get_username();

    unsigned char hash[crypto_hash_sha256_BYTES];
    auto *data_c_style = new unsigned char[data.size()];

    for (int i = 0; i < data.size(); ++i) {
        data_c_style[i] = static_cast<unsigned char>(data[i]);
    }

    crypto_hash_sha256(hash, data_c_style, data.size());

    std::ostringstream result;
    for (unsigned char byte : hash) {
        result << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return result.str();
}

time_t Session::generate_expired_at() {
    return std::time(nullptr) + static_cast<time_t>(Config::config()["bounds"]["valid_session_time"]);
}

} // namespace oink_judge::backend::auth
