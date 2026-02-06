#pragma once
#include "oink_judge/dispatcher/problem_submission_manager.hpp"

#include <oink_judge/socket/protocols/protocol_base.h>

namespace oink_judge::dispatcher {

using boost::asio::awaitable;

class ProtocolWithFastAPI : public socket::ProtocolBase {
  public:
    ProtocolWithFastAPI();

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithFastAPI";

  private:
    std::map<std::string, std::shared_ptr<ProblemSubmissionManager>> submission_managers_;
};

} // namespace oink_judge::dispatcher
