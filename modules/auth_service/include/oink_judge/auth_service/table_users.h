#pragma once
#include <string>

namespace oink_judge::auth_service {

class TableUsers {
  public:
    static auto instance() -> TableUsers&;

    TableUsers(const TableUsers&) = delete;
    auto operator=(const TableUsers&) -> TableUsers& = delete;
    TableUsers(TableUsers&&) = delete;
    auto operator=(TableUsers&&) -> TableUsers& = delete;
    ~TableUsers() = default;

    auto authenticate(const std::string& username, const std::string& password) -> bool;
    auto registerUser(const std::string& username, const std::string& password) -> bool;
    auto userExists(const std::string& username) -> bool;
    auto deleteUser(const std::string& username) -> bool;
    auto updatePassword(const std::string& username, const std::string& new_password) -> bool;

  private:
    TableUsers();
};

} // namespace oink_judge::auth_service
