#include "oink_judge/content_service/content_client_protocol.h"

#include <oink_judge/config/config.h>
#include <oink_judge/utils/crypto.h>

namespace oink_judge::content_service {

using json = nlohmann::json;
using Config = config::Config;

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().registerType(ContentClientProtocol::REGISTERED_NAME,
                                                     [](const std::string& params) -> std::unique_ptr<ContentClientProtocol> {
                                                         return std::make_unique<ContentClientProtocol>();
                                                     });

    return true;
}();

} // namespace

ContentClientProtocol::ContentClientProtocol() = default;

auto ContentClientProtocol::start(std::string start_message) -> awaitable<void> {
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ContentClientProtocol::receiveMessage(std::string message) -> awaitable<void> {
    json received_json = json::parse(message);

    if (received_json["request"] == "response") {
        uint64_t response_id = received_json["__id__"];
        auto callback_opt = retrieveRequestCallback(response_id);
        if (!callback_opt) {
            co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
            co_return;
        }
        auto callback = *callback_opt;
        if (received_json["sended_request"] == "get_manifest") {
            std::decay_t<json> manifest = received_json["manifest"];
            callCallback<json>(callback, std::error_code{}, manifest);
        } else if (received_json["sended_request"] == "get_file") {
            std::string encoded_file = received_json["file_content"];
            std::string decoded_file = utils::crypto::fromBase64(encoded_file);
            callCallback<std::string>(callback, std::error_code{}, decoded_file);
        } else if (received_json["sended_request"] == "update_file" || received_json["sended_request"] == "remove_file") {
            callCallback(callback, std::error_code{});
        }
        removeRequestCallback(response_id);
    }

    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ContentClientProtocol::closeSession() -> void {
    socket::ProtocolWithRequests::closeSession();
    // Reconnect to the server
}

} // namespace oink_judge::content_service
