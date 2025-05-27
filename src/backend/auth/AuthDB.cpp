#include "backend/auth/AuthDB.h"
#include <stdexcept>
#include <cstring>
#include <sodium.h>
#include <iostream>

namespace oink_judge::backend::auth {

namespace {
    const char *hash_password(const char *password) {
        unsigned char password_hash_tmp[crypto_generichash_BYTES];
        crypto_generichash(password_hash_tmp, sizeof password_hash_tmp,
                       (const unsigned char*)(password), strlen(password),
                       NULL, 0);

        static char password_hash[crypto_generichash_BYTES + 1];
        for (size_t i = 0; i < crypto_generichash_BYTES; ++i) {
            password_hash[i] = password_hash_tmp[i];
        }
        password_hash[crypto_generichash_BYTES] = '\0';
        return password_hash;
    }
} // namespace

AuthDB::AuthDB(const char *db_path) {
    int rc = sqlite3_open(db_path, &_db);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(_db)));
    }

    const char* create_sql = "CREATE TABLE IF NOT EXISTS users ("
                             "username TEXT PRIMARY KEY,"
                             "password TEXT);";

    char *err_msg = nullptr;
    rc = sqlite3_exec(_db, create_sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Failed to create table: " + std::string(err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(_db);
        throw std::runtime_error(error_msg);
    }

    sqlite3_free(err_msg);
}

AuthDB::~AuthDB() {
    if (_db) {
        sqlite3_close(_db);
    }
}

bool AuthDB::authenticate(const char *username, const char *password) {
    const char *password_hash = hash_password(password);

    sqlite3_stmt *stmt;
    const char *sql = "SELECT password FROM users WHERE username = ?";

    int rc = sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    bool authenticated = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_password_hash = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        authenticated = (stored_password_hash && strcmp(stored_password_hash, password_hash) == 0);
    }

    sqlite3_finalize(stmt);
    return authenticated;
}

bool AuthDB::register_user(const char *username, const char *password) {
    if (user_exists(username)) {
        return false; // User already exists
    }

    const char *password_hash = hash_password(password);

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?)";

    int rc = sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AuthDB::user_exists(const char *username) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM users WHERE username = ?";

    int rc = sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return exists;
}

bool AuthDB::delete_user(const char *username) {
    if (!user_exists(username)) {
        return false; // User does not exist
    }

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM users WHERE username = ?";

    int rc = sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AuthDB::update_password(const char *username, const char *new_password) {
    if (!user_exists(username)) {
        return false; // User does not exist
    }

    const char *password_hash = hash_password(new_password);

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE users SET password = ? WHERE username = ?";

    int rc = sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    sqlite3_bind_text(stmt, 1, password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

} // namespace oink_judge::backend::auth
