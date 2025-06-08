#include "backend/auth/Session.h"
#include <ctime>

namespace oink_judge::backend::auth {

Session::Session(const std::string &username) : _username(username) {
    _max_inactivity_time = 3600; // 1 hour
    _last_activity = std::time(nullptr);
    if (username.empty()) {
        _session_id = "";
    } else {
        _session_id = std::to_string(std::time(nullptr)) + "-" + std::to_string(rand()) + "-" + _username;
    }
}

bool Session::is_valid() const {
    time_t current_time = std::time(nullptr);
    return (current_time - _last_activity) <= _max_inactivity_time;
}

const std::string& Session::get_session_id() const {
    if (!is_valid()) {
        return "";
    }

    _last_activity = std::time(nullptr);

    return _session_id;
}

const std::string& Session::get_username() const {
    return _username;
}

void Session::invalidate() {
    _session_id.clear();
    _username.clear();
}

} // namespace oink_judge::backend::auth
