#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace oink_judge::auth_service {

auto handleClient(const json& data) -> json;

auto handleLoginRequest(const json& data) -> json;

auto handleWhoseSessionRequest(const json& data) -> json;

auto handleRegisterRequest(const json& data) -> json;

auto handleDeleteUserRequest(const json& data) -> json;

auto handleUpdatePasswordRequest(const json& data) -> json;

auto handleLogoutRequest(const json& data) -> json;

} // namespace oink_judge::auth_service
