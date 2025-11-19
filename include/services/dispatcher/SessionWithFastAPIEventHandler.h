#pragma once

#include "socket/SessionEventHandler.hpp"
#include "ProblemSubmissionManager.hpp"

namespace oink_judge::services::dispatcher {

class SessionWithFastAPIEventHandler : public socket::SessionEventHandler {
public:
    SessionWithFastAPIEventHandler();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<socket::Session> session) override;
    std::shared_ptr<socket::Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "SessionWithFastAPIEventHandler";

    void request_internal(const std::string &message, const callback_t &callback) override;

private:
    std::weak_ptr<socket::Session> _session;

    std::map<std::string, std::shared_ptr<ProblemSubmissionManager>> _submission_managers;
};

} // namespace oink_judge::services::dispatcher
