#pragma once

#include <nlohmann/json.hpp>
#include "ParameterizedTypeFactory.h"
#include "Session.hpp"

namespace oink_judge::socket {

class SessionEventHandler {
public:
    virtual ~SessionEventHandler() = default;

    virtual void start(const std::string &start_message) = 0;
    virtual void receive_message(const std::string &message) = 0;
    virtual void close_session() = 0;

    virtual void set_session(std::weak_ptr<Session> session) = 0;
    virtual std::weak_ptr<Session> get_session() const = 0;
};

using BasicSessionEventHandlerFactory = ParameterizedTypeFactory<std::unique_ptr<SessionEventHandler>>;

} // namespace oink_judge::socket
