#include "oink_judge/auth_service/table_users.h"

#include <iostream>
#include <oink_judge/database/database.h>
#include <oink_judge/utils/crypto.h>
#include <string>

namespace oink_judge::auth_service {

using DataBase = database::DataBase;

auto TableUsers::instance() -> TableUsers& {
    static TableUsers instance;
    return instance;
}

TableUsers::TableUsers() {
    const std::string CREATE_SQL = "CREATE TABLE IF NOT EXISTS users ("
                                   "username TEXT PRIMARY KEY,"
                                   "password TEXT);";

    DataBase::instance().executeSQL(CREATE_SQL);

    const std::string SELECT_PASSWORD_SQL = "SELECT password FROM users WHERE username = $1";
    const std::string INSERT_USER_SQL = "INSERT INTO users (username, password) VALUES ($1, $2)";
    const std::string DELETE_USER_SQL = "DELETE FROM users WHERE username = $1";
    const std::string UPDATE_USER_SQL = "UPDATE users SET password = $2 WHERE username = $1";

    DataBase::instance().prepareStatement("users__select_password", SELECT_PASSWORD_SQL);
    DataBase::instance().prepareStatement("users__insert_user", INSERT_USER_SQL);
    DataBase::instance().prepareStatement("users__delete_user", DELETE_USER_SQL);
    DataBase::instance().prepareStatement("users__update_user_password", UPDATE_USER_SQL);
}

auto TableUsers::authenticate(const std::string& username, const std::string& password) -> bool { // NOLINT
    std::string password_hash = utils::crypto::sha256(password);

    try {
        pqxx::result res = DataBase::instance().executeReadOnly("users__select_password", username);

        if (res.empty()) {
            return false;
        }

        if (res.size() > 1) {
            throw std::runtime_error("User must have exactly one password");
        }

        auto stored_password_hash = res[0]["password"].as<std::string>();

        return stored_password_hash == password_hash;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableUsers::registerUser(const std::string& username, const std::string& password) -> bool {
    if (userExists(username)) {
        return false; // User already exists
    }

    std::string password_hash = utils::crypto::sha256(password);

    try {
        DataBase::instance().execute("users__insert_user", username, password_hash);

        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableUsers::userExists(const std::string& username) -> bool { // NOLINT
    try {
        pqxx::result res = DataBase::instance().executeReadOnly("users__select_password", username);

        return !res.empty();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableUsers::deleteUser(const std::string& username) -> bool {
    if (!userExists(username)) {
        return false; // User does not exist
    }

    try {
        DataBase::instance().execute("users__delete_user", username);

        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableUsers::updatePassword(const std::string& username, const std::string& new_password) -> bool {
    if (!userExists(username)) {
        return false; // User does not exist
    }

    std::string password_hash = utils::crypto::sha256(new_password);

    try {
        DataBase::instance().execute("users__update_user_password", username, password_hash);

        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

} // namespace oink_judge::auth_service
