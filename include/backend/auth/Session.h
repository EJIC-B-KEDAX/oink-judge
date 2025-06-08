#pragma once

#include <string>

namespace oink_judge::backend::auth {

class Session {
public:
    Session(const std::string &username = "");

    bool is_valid() const;

    const std::string& get_session_id() const;

    const std::string& get_username() const;

    void invalidate();

private:
    std::string _session_id;
    std::string _username;
    mutable time_t _last_activity;
    time_t _max_inactivity_time;
};

} // namespace oink_judge::backend::auth
