#pragma once

#include "socket/protocols/ProtocolBase.h"
#include "ProblemSubmissionManager.hpp"

namespace oink_judge::services::dispatcher {

using boost::asio::awaitable;

class ProtocolWithFastAPI : public socket::ProtocolBase {
public:
    ProtocolWithFastAPI();

    awaitable<void> start(const std::string &start_message) override;
    awaitable<void> receive_message(const std::string &message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithFastAPI";

private:
    std::map<std::string, std::shared_ptr<ProblemSubmissionManager>> _submission_managers;
};

} // namespace oink_judge::services::dispatcher
