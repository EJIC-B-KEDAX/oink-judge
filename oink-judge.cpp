#include <iostream>
#include "backend/auth/AuthDB.h"

int32_t main() {
    oink_judge::backend::auth::AuthDB auth_db("../users.db");
    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;
    if (auth_db.authenticate(username.c_str(), password.c_str())) {
        std::cout << "User authenticate successfully." << std::endl;
    } else {
        std::cout << "Failed to authenticate user." << std::endl;
    }
}