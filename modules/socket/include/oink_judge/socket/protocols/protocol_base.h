#pragma once
#include "socket/Protocol.hpp"

namespace oink_judge::socket {

class ProtocolBase : public Protocol {
  public:
    ProtocolBase() = default;

    awaitable<void> send_message(std::string message) override;

    void set_session(std::weak_ptr<socket::Session> session) override;
    std::shared_ptr<socket::Session> get_session() const override;

    void request_internal(std::string message, const callback_t& callback) override;

  private:
    std::weak_ptr<socket::Session> _session;
};

} // namespace oink_judge::socket
