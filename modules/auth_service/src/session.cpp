#include "oink_judge/auth_service/session.h"

#include <ctime>
#include <oink_judge/config/config.h>
#include <oink_judge/utils/crypto.h>
#include <random>

namespace oink_judge::auth_service {

namespace {

std::random_device rd;     // NOLINT
std::mt19937_64 rng(rd()); // NOLINT

} // namespace

using Config = config::Config;

Session::Session(std::string username) : username_(std::move(username)), expire_at_(0) {}

auto Session::generateSession() -> void {
    if (!session_id_.empty()) {
        return;
    }

    session_id_ = generateSessionId();
    expire_at_ = generateExpiredAt();
}

auto Session::isValid() const -> bool {
    time_t current_time = std::time(nullptr);
    return current_time <= expire_at_;
}

auto Session::getSessionId() const -> std::string {
    if (!isValid()) {
        return "";
    }

    return session_id_;
}

auto Session::getUsername() const -> const std::string& { return username_; }

auto Session::getExpireAt() const -> time_t { return expire_at_; }

auto Session::generateSessionId() -> std::string {
    time_t current_time = std::time(nullptr);
    std::string data = std::to_string(current_time) + "." + std::to_string(rng()) + "." + getUsername();

    return utils::crypto::sha256(data);
}

auto Session::generateExpiredAt() -> time_t {
    return std::time(nullptr) + static_cast<time_t>(Config::config()["bounds"]["valid_session_time"]);
}

} // namespace oink_judge::auth_service
