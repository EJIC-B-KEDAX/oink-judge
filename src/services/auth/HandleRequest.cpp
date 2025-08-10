#include <services/auth/HandleRequest.h>
#include "services/auth/AuthManager.h"

namespace oink_judge::services::auth {

json handle_client(const json &data) {
    if (data["request"] == "login") {
        return handle_login_request(data);
    } else if (data["request"] == "whose_session") {
        return handle_whose_session_request(data);
    } else if (data["request"] == "register") {
        return handle_register_request(data);
    } else if (data["request"] == "delete_user") {
        return handle_delete_user_request(data);
    } else if (data["request"] == "update_password") {
        return handle_update_password_request(data);
    } else if (data["request"] == "logout") {
        return handle_logout_request(data);
    }

    return json{{"status", "error"}, {"message", "Unknown request type" + std::string(data["request"])}};
}

json handle_login_request(const json &data) {

    std::string username = data["username"]; 
    std::string password = data["password"];
    std::string session_id = AuthManager::instance().authenticate(username, password);

    json response;
    if (!session_id.empty()) {
        response["status"] = "success";
        response["session_id"] = session_id;
    } else {
        response["status"] = "error";
        response["message"] = "Invalid credentials";
    }

    return response;
}

json handle_whose_session_request(const json &data) {
    std::string session_id = data["session_id"];
    std::string username = AuthManager::instance().whose_session(session_id);

    json response;
    if (!username.empty()) {
        response["status"] = "success";
        response["username"] = username;
    } else {
        response["status"] = "error";
        response["message"] = "Invalid session ID";
    }

    return response;
}

json handle_register_request(const json &data) {
    std::string username = data["username"];
    std::string password = data["password"];
    bool success = AuthManager::instance().register_user(username, password);

    json response;
    if (success) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
        response["message"] = "Registration failed";
    }

    return response;
}

json handle_delete_user_request(const json &data) {
    std::string username = data["username"];
    bool success = AuthManager::instance().delete_user(username);

    json response;
    if (success) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
        response["message"] = "User deletion failed or user does not exist";
    }

    return response;
}

json handle_update_password_request(const json &data) {
    std::string username = data["username"];
    std::string new_password = data["new_password"];
    bool success = AuthManager::instance().update_password(username, new_password);

    json response;
    if (success) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
        response["message"] = "Password update failed or user does not exist";
    }

    return response;
}

json handle_logout_request(const json &data) {
    std::string session_id = data["session_id"];
    AuthManager::instance().invalidate_session(session_id);

    json response;
    response["status"] = "success";
    return response;
}

} // namespace oink_judge::services::auth
