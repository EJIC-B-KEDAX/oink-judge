#pragma once
#include "oink_judge/socket/protocol.hpp"

namespace oink_judge::socket {

class ProtocolDecorator : public Protocol {
  public:
    ProtocolDecorator(std::unique_ptr<Protocol> inner_protocol);

    auto start(std::string start_message) -> awaitable<void> override;
    auto sendMessage(std::string message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    auto setSession(std::weak_ptr<Session> session) -> void override;
    [[nodiscard]] auto getSession() const -> std::shared_ptr<Session> override;

    auto requestInternal(const std::string& message, const callback_t& callback) -> void override;

  protected:
    auto accessInnerProtocol() -> std::unique_ptr<Protocol>&;

  private:
    std::unique_ptr<Protocol> inner_protocol_;
};

} // namespace oink_judge::socket
