#include "oink_judge/auth_service/handle_request.h"

#include "oink_judge/auth_service/auth_manager.h"

namespace oink_judge::auth_service {

auto handleClient(const json& data) -> json {
    if (data["request"] == "login") {
        return handleLoginRequest(data);
    }
    if (data["request"] == "whose_session") {
        return handleWhoseSessionRequest(data);
    }
    if (data["request"] == "register") {
        return handleRegisterRequest(data);
    }
    if (data["request"] == "delete_user") {
        return handleDeleteUserRequest(data);
    }
    if (data["request"] == "update_password") {
        return handleUpdatePasswordRequest(data);
    }
    if (data["request"] == "logout") {
        return handleLogoutRequest(data);
    }

    return json{
        {"__id__", data.at("__id__")}, {"status", "error"}, {"message", "Unknown request type" + std::string(data["request"])}};
}

auto handleLoginRequest(const json& data) -> json {
    std::string username = data["username"];
    std::string password = data["password"];
    std::string session_id = AuthManager::instance().authenticate(username, password);

    json response;
    response["__id__"] = data.at("__id__");
    if (!session_id.empty()) {
        response["status"] = "success";
        response["session_id"] = session_id;
    } else {
        response["status"] = "error";
        response["message"] = "Invalid credentials";
    }

    return response;
}

auto handleWhoseSessionRequest(const json& data) -> json {
    std::string session_id = data["session_id"];
    std::string username = AuthManager::instance().whoseSession(session_id);

    json response;
    response["__id__"] = data.at("__id__");
    if (!username.empty()) {
        response["status"] = "success";
        response["username"] = username;
    } else {
        response["status"] = "error";
        response["message"] = "Invalid session ID";
    }

    return response;
}

auto handleRegisterRequest(const json& data) -> json {
    std::string username = data["username"];
    std::string password = data["password"];
    bool success = AuthManager::instance().registerUser(username, password);

    json response;
    response["__id__"] = data.at("__id__");
    if (success) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
        response["message"] = "Registration failed";
    }

    return response;
}

auto handleDeleteUserRequest(const json& data) -> json {
    std::string username = data["username"];
    bool success = AuthManager::instance().deleteUser(username);

    json response;
    response["__id__"] = data.at("__id__");
    if (success) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
        response["message"] = "User deletion failed or user does not exist";
    }

    return response;
}

auto handleUpdatePasswordRequest(const json& data) -> json {
    std::string username = data["username"];
    std::string new_password = data["new_password"];
    bool success = AuthManager::instance().updatePassword(username, new_password);

    json response;
    response["__id__"] = data.at("__id__");
    if (success) {
        response["status"] = "success";
    } else {
        response["status"] = "error";
        response["message"] = "Password update failed or user does not exist";
    }

    return response;
}

auto handleLogoutRequest(const json& data) -> json {
    std::string session_id = data["session_id"];
    AuthManager::instance().invalidateSession(session_id);

    json response;
    response["__id__"] = data.at("__id__");
    response["status"] = "success";
    return response;
}

} // namespace oink_judge::auth_service
