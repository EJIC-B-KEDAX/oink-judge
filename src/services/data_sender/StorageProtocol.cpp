#include "services/data_sender/StorageProtocol.h"
#include "config/Config.h"
#include "utils/crypto.h"
#include <iostream>

namespace oink_judge::services::data_sender {

using json = nlohmann::json;
using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::ProtocolFactory::instance().register_type(StorageProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<StorageProtocol> {
        return std::make_unique<StorageProtocol>();
    });

    return true;
}();

} // namespace

StorageProtocol::StorageProtocol() {};

awaitable<void> StorageProtocol::start(const std::string &start_message) {
    co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
}

awaitable<void> StorageProtocol::receive_message(const std::string &message) {
    std::cerr << "StorageProtocol received message: " << message << std::endl;
    json received_json = json::parse(message);

    if (received_json["request"] == "response") {
        uint64_t response_id = received_json["__id__"];
        std::cerr << "Processing response with ID: " << response_id << std::endl;
        auto callback_opt = retrieve_request_callback(response_id);
        if (!callback_opt) {
            co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
            co_return;
        }
        auto callback = *callback_opt;
        if (received_json["sended_request"] == "get_manifest") {
            std::decay_t<json> manifest = received_json["manifest"];
            std::cerr << "Sending manifest in response: " << manifest.dump() << std::endl;
            call_callback<json>(callback, std::error_code{}, manifest);
        } else if (received_json["sended_request"] == "get_file") {
            std::string encoded_file = received_json["file_content"];
            std::string decoded_file = utils::crypto::from_base64(encoded_file);
            call_callback<std::string>(callback, std::error_code{}, decoded_file);
        } else if (received_json["sended_request"] == "update_file" ||
                   received_json["sended_request"] == "remove_file") {
            call_callback(callback, std::error_code{});
        }
        remove_request_callback(response_id);
    }
    
    co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
}

void StorageProtocol::close_session() {
    socket::ProtocolWithRequests::close_session();
    // Reconnect to the server
}

} // namespace oink_judge::services::data_sender
