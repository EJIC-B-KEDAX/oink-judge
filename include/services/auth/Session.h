#pragma once
#include <string>

namespace oink_judge::services::auth {

class Session {
  public:
    Session(const std::string& username);
    virtual ~Session() = default;

    void generate_session();

    bool is_valid() const;

    std::string get_session_id() const;

    const std::string& get_username() const;

    time_t get_expire_at() const;

  protected:
    virtual std::string generate_session_id();
    virtual time_t generate_expired_at();

  private:
    std::string _session_id;
    std::string _username;
    time_t _expire_at;
};

} // namespace oink_judge::services::auth
