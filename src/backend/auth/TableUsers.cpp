#include "backend/auth/TableUsers.h"
#include "database/DataBase.h"
#include <cstring>
#include <string>
#include <sodium.h>
#include <iostream>

namespace oink_judge::backend::auth {

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

        std::ostringstream result;
        for (unsigned char byte : password_hash) {
            result << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return result.str();
    }
} // namespace

TableUsers &TableUsers::instance() {
    static TableUsers instance;
    return instance;
}


TableUsers::TableUsers() {
    const std::string create_sql = "CREATE TABLE IF NOT EXISTS users ("
                                       "username TEXT PRIMARY KEY,"
                                       "password TEXT);";

    DataBase::instance().execute_sql(create_sql);

    const std::string select_password_sql = "SELECT password FROM users WHERE username = $1";

    const std::string insert_user_sql = "INSERT INTO users (username, password) VALUES ($1, $2)";

    const std::string delete_user_sql = "DELETE FROM users WHERE username = $1";

    const std::string update_user_sql = "UPDATE users SET password = $2 WHERE username = $1";

    DataBase::instance().prepare_statement("users__select_password", select_password_sql);
    DataBase::instance().prepare_statement("users__insert_user", insert_user_sql);
    DataBase::instance().prepare_statement("users__delete_user", delete_user_sql);
    DataBase::instance().prepare_statement("users__update_user_password", update_user_sql);
}

bool TableUsers::authenticate(const std::string &username, const std::string &password) {
    std::string password_hash = hash_password(password);

    try {
        pqxx::result res = DataBase::instance().execute_read_only("users__select_password", username);

        if (res.empty()) return false;

        if (res.size() > 1) {
            throw std::runtime_error("User must have exactly one password");
        }

        auto stored_password_hash = res[0]["password"].as<std::string>();

        if (stored_password_hash == password_hash) return true;

        return false;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool TableUsers::register_user(const std::string &username, const std::string &password) {
    if (user_exists(username)) {
        return false; // User already exists
    }

    std::string password_hash = hash_password(password);

    try {
        DataBase::instance().execute("users__insert_user", username, password_hash);

        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool TableUsers::user_exists(const std::string &username) {
    try {
        pqxx::result res = DataBase::instance().execute_read_only("users__select_password", username);

        return !res.empty();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool TableUsers::delete_user(const std::string &username) {
    if (!user_exists(username)) {
        return false; // User does not exist
    }

    try {
        DataBase::instance().execute("users__delete_user", username);

        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool TableUsers::update_password(const std::string &username, const std::string &new_password) {
    if (!user_exists(username)) {
        return false; // User does not exist
    }

    std::string password_hash = hash_password(new_password);

    try {
        DataBase::instance().execute("users__update_user_password", username, password_hash);

        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace oink_judge::backend::auth
