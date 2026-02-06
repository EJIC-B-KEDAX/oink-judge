#pragma once
#include "oink_judge/socket/session.hpp"

#include <nlohmann/json.hpp>
#include <oink_judge/factory/parameterized_type_factory.hpp>

namespace oink_judge::socket {

class Protocol {
  public:
    using callback_t = std::function<void(std::error_code, std::any)>;

    Protocol(const Protocol&) = delete;
    auto operator=(const Protocol&) -> Protocol& = delete;
    Protocol(Protocol&&) = delete;
    auto operator=(Protocol&&) -> Protocol& = delete;
    virtual ~Protocol() = default;

    virtual auto start(std::string start_message) -> awaitable<void> = 0; // TODO: remove links to string
    virtual auto sendMessage(std::string message) -> awaitable<void> = 0;
    virtual auto receiveMessage(std::string message) -> awaitable<void> = 0;
    virtual auto closeSession() -> void = 0;

    virtual auto setSession(std::weak_ptr<Session> session) -> void = 0;
    [[nodiscard]] virtual auto getSession() const -> std::shared_ptr<Session> = 0;

    virtual auto requestInternal(const std::string& message, const callback_t& callback) -> void = 0;

  protected:
    Protocol() = default;

    template <typename... Args>
    static auto callCallback(const callback_t& callback, std::error_code ec, std::decay_t<Args>... args) -> void;
};

using ProtocolFactory = factory::ParameterizedTypeFactory<std::unique_ptr<Protocol>>;

} // namespace oink_judge::socket

#include "oink_judge/socket/protocol.inl"
