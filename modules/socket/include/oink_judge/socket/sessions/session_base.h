#pragma once
#include "oink_judge/socket/protocol.hpp"
#include "oink_judge/socket/session.hpp"

namespace oink_judge::socket {

class SessionBase : public Session, public std::enable_shared_from_this<Session> {
  public:
    auto setSessionPtr() -> void;
    auto requestInternal(const std::string& message, const callback_t& callback) -> void override;

  protected:
    SessionBase(std::unique_ptr<Protocol> protocol);

    auto accessProtocol() -> Protocol&;

  private:
    std::unique_ptr<Protocol> protocol_;
};

} // namespace oink_judge::socket
