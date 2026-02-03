#pragma once
#include "ParameterizedTypeFactory.hpp"
#include "Session.hpp"

#include <nlohmann/json.hpp>

namespace oink_judge::socket {

class Protocol {
  public:
    using callback_t = std::function<void(std::error_code, std::any)>;

    virtual ~Protocol() = default;

    virtual awaitable<void> start(std::string start_message) = 0; // TODO: remove links to string
    virtual awaitable<void> send_message(std::string message) = 0;
    virtual awaitable<void> receive_message(std::string message) = 0;
    virtual void close_session() = 0;

    virtual void set_session(std::weak_ptr<Session> session) = 0;
    virtual std::shared_ptr<Session> get_session() const = 0;

    virtual void request_internal(std::string message, const callback_t& callback) = 0;

  protected:
    template <typename... Args>
    static void call_callback(const callback_t& callback, std::error_code ec, std::decay_t<Args>... args);
};

using ProtocolFactory = ParameterizedTypeFactory<std::unique_ptr<Protocol>>;

} // namespace oink_judge::socket

#include "Protocol.inl"
