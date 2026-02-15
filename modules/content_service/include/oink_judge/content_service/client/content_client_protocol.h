#pragma once
#include <oink_judge/socket/protocols/protocol_with_requests.h>
#include <string>

namespace oink_judge::content_service {

using boost::asio::awaitable;
using Session = socket::Session;

class ContentClientProtocol : public socket::ProtocolWithRequests {
  public:
    ContentClientProtocol();

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    constexpr static auto REGISTERED_NAME = "ContentClientProtocol";
};

} // namespace oink_judge::content_service
