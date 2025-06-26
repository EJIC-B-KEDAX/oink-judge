#pragma once

#include <string>

namespace oink_judge::backend::auth {

class TableUsers {
public:
    static TableUsers &instance();

    TableUsers(const TableUsers &) = delete;
    TableUsers &operator=(const TableUsers &) = delete;

    bool authenticate(const std::string &username, const std::string &password);
    bool register_user(const std::string &username, const std::string &password);
    bool user_exists(const std::string &username);
    bool delete_user(const std::string &username);
    bool update_password(const std::string &username, const std::string &new_password);

private:
    TableUsers();
};

} // namespace oink_judge::backend::auth
