#pragma once
#include "oink_judge/socket/protocol.hpp"

namespace oink_judge::socket {

class ProtocolBase : public Protocol {
  public:
    auto sendMessage(std::string message) -> awaitable<void> override;

    auto setSession(std::weak_ptr<socket::Session> session) -> void override;
    [[nodiscard]] auto getSession() const -> std::shared_ptr<socket::Session> override;

    auto requestInternal(const std::string& message, const callback_t& callback) -> void override;

  private:
    std::weak_ptr<socket::Session> session_;
};

} // namespace oink_judge::socket
