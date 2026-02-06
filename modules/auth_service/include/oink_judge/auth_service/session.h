#pragma once
#include <string>

namespace oink_judge::auth_service {

class Session {
  public:
    Session(std::string username);

    Session(const Session&) = default;
    auto operator=(const Session&) -> Session& = default;
    Session(Session&&) = default;
    auto operator=(Session&&) -> Session& = default;
    virtual ~Session() = default;

    auto generateSession() -> void;
    [[nodiscard]] auto isValid() const -> bool;
    [[nodiscard]] auto getSessionId() const -> std::string;
    [[nodiscard]] auto getUsername() const -> const std::string&;
    [[nodiscard]] auto getExpireAt() const -> time_t;

  protected:
    virtual auto generateSessionId() -> std::string;
    virtual auto generateExpiredAt() -> time_t;

  private:
    std::string session_id_;
    std::string username_;
    time_t expire_at_;
};

} // namespace oink_judge::auth_service
