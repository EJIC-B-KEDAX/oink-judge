#pragma once

#include <sqlite3.h>

namespace oink_judge::backend::auth {

class AuthDB {
public:
    AuthDB(const char *db_path);
    ~AuthDB();

    AuthDB(const AuthDB &) = delete;
    AuthDB &operator=(const AuthDB &) = delete;
    AuthDB(AuthDB &&) = delete;
    AuthDB &operator=(AuthDB &&) = delete;

    bool authenticate(const char *username, const char *password);
    bool register_user(const char *username, const char *password);
    bool user_exists(const char *username);
    bool delete_user(const char *username);
    bool update_password(const char *username, const char *new_password);

private:
    sqlite3 *_db;
};

} // namespace oink_judge::backend::auth
