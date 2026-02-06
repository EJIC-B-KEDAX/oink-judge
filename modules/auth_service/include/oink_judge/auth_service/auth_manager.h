#pragma once
#include <string>

namespace oink_judge::auth_service {

class AuthManager {
  public:
    static auto instance() -> AuthManager&;

    AuthManager(const AuthManager&) = delete;
    auto operator=(const AuthManager&) -> AuthManager& = delete;
    AuthManager(AuthManager&&) = delete;
    auto operator=(AuthManager&&) -> AuthManager& = delete;
    ~AuthManager() = default;

    auto registerUser(const std::string& username, const std::string& password) -> bool;
    auto userExists(const std::string& username) -> bool;
    auto deleteUser(const std::string& username) -> bool;
    auto updatePassword(const std::string& username, const std::string& new_password) -> bool;

    auto authenticate(const std::string& username, const std::string& password) -> std::string;
    auto whoseSession(const std::string& session_id) -> std::string;
    auto isSessionValid(const std::string& session_id) -> bool;
    auto invalidateSession(const std::string& session_id) -> void;

  private:
    AuthManager() = default;
};

} // namespace oink_judge::auth_service
