#pragma once

#include <nlohmann/json.hpp>
#include "ParameterizedTypeFactory.h"
#include "Session.hpp"

namespace oink_judge::socket {

class SessionEventHandler {
public:
    using callback_t = std::function<void(std::error_code, std::any)>;
    
    virtual ~SessionEventHandler() = default;

    virtual void start(const std::string &start_message) = 0;
    virtual void receive_message(const std::string &message) = 0;
    virtual void close_session() = 0;

    virtual void set_session(std::weak_ptr<Session> session) = 0;
    virtual std::shared_ptr<Session> get_session() const = 0;

    template<typename F, typename... Args>
    void request(const std::string &message, F&& callback);

    virtual void request_internal(const std::string &message, const callback_t &callback) = 0;

protected:
    template<typename... Args>
    static void call_callback(const callback_t &callback, std::error_code ec, Args&&... args);
};

using BasicSessionEventHandlerFactory = ParameterizedTypeFactory<std::unique_ptr<SessionEventHandler>>;

} // namespace oink_judge::socket

#include "SessionEventHandler.inl"
