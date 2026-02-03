#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace oink_judge::services::auth {

json handle_client(const json& data);

json handle_login_request(const json& data);

json handle_whose_session_request(const json& data);

json handle_register_request(const json& data);

json handle_delete_user_request(const json& data);

json handle_update_password_request(const json& data);

json handle_logout_request(const json& data);

} // namespace oink_judge::services::auth
