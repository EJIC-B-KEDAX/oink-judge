#include "backend/auth/TableUsers.h"
#include "database/DataBase.h"
#include <cstring>
#include <string>
#include <sodium.h>
#include <iostream>

namespace oink_judge::backend::auth {

using Statement = database::Statement;
using DataBase = database::DataBase;

namespace {
    std::string hash_password(const std::string &password) {
        unsigned char password_hash_tmp[crypto_generichash_BYTES];
        crypto_generichash(password_hash_tmp, sizeof password_hash_tmp,
                       reinterpret_cast<const unsigned char *>(password.c_str()), password.size(),
                       nullptr, 0);

        static char password_hash[crypto_generichash_BYTES + 1];
        for (size_t i = 0; i < crypto_generichash_BYTES; ++i) {
            password_hash[i] = static_cast<char>(password_hash_tmp[i]);
        }
        password_hash[crypto_generichash_BYTES] = '\0';
        return password_hash;
    }
} // namespace

TableUsers &TableUsers::instance() {
    static TableUsers instance;
    return instance;
}


TableUsers::TableUsers() {
    std::string create_sql = "CREATE TABLE IF NOT EXISTS users ("
                             "username TEXT PRIMARY KEY,"
                             "password TEXT);";

    Statement stmt;
    DataBase::instance().prepare_statement(stmt, create_sql);

    stmt.step();
}

bool TableUsers::authenticate(const std::string &username, const std::string &password) {
    std::string password_hash = hash_password(password);

    Statement stmt;
    std::string sql = "SELECT password FROM users WHERE username = ?";

    DataBase::instance().prepare_statement(stmt, sql, username);

    bool authenticated = false;
    if (stmt.step() == SQLITE_ROW) {
        std::string stored_password_hash = stmt.column_text(0);
        authenticated = !stored_password_hash.empty() && stored_password_hash == password_hash;
    }

    return authenticated;
}

bool TableUsers::register_user(const std::string &username, const std::string &password) {
    if (user_exists(username)) {
        return false; // User already exists
    }

    std::string password_hash = hash_password(password);

    Statement stmt;
    std::string sql = "INSERT INTO users (username, password) VALUES (?, ?)";

    DataBase::instance().prepare_statement(stmt, sql, username, password_hash);

    return stmt.step() == SQLITE_DONE;
}

bool TableUsers::user_exists(const std::string &username) {
    Statement stmt;
    std::string sql = "SELECT 1 FROM users WHERE username = ?";

    DataBase::instance().prepare_statement(stmt, sql, username);

    return stmt.step() == SQLITE_ROW;
}

bool TableUsers::delete_user(const std::string &username) {
    if (!user_exists(username)) {
        return false; // User does not exist
    }

    Statement stmt;
    std::string sql = "DELETE FROM users WHERE username = ?";

    DataBase::instance().prepare_statement(stmt, sql, username);

    return stmt.step() == SQLITE_DONE;
}

bool TableUsers::update_password(const std::string &username, const std::string &new_password) {
    if (!user_exists(username)) {
        return false; // User does not exist
    }

    std::string password_hash = hash_password(new_password);

    Statement stmt;
    std::string sql = "UPDATE users SET password = ? WHERE username = ?";

    DataBase::instance().prepare_statement(stmt, sql, password_hash, username);

    return stmt.step() == SQLITE_DONE;
}

} // namespace oink_judge::backend::auth
