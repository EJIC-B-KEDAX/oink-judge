#include "oink_judge/socket/protocols/protocol_with_requests.h"

#include <nlohmann/json.hpp>
#include <oink_judge/logger/logger.h>

namespace oink_judge::socket {

using json = nlohmann::json;

auto ProtocolWithRequests::requestInternal(const std::string& message, const callback_t& callback) -> void {
    static uint64_t request_id_counter = 0;
    uint64_t request_id = ++request_id_counter;

    storeRequestCallback(request_id, callback);

    json request_json;

    try {
        request_json = json::parse(message);
    } catch (const json::parse_error& e) {
        request_json = json::object();
        request_json["data"] = message;
    }

    request_json["__id__"] = request_id;

    boost::asio::co_spawn(getSession()->getExecutor(), sendMessage(request_json.dump()), boost::asio::detached);
}

auto ProtocolWithRequests::closeSession() -> void {
    for (auto& [request_id, callback] : pending_requests_callbacks_) {
        logger::logMessage("ProtocolWithRequests", 1,
                           "Cancelling pending request with ID " + std::to_string(request_id) + " due to session closure",
                           logger::WARNING);
        callCallback(callback, std::make_error_code(std::errc::operation_canceled));
    }
    pending_requests_callbacks_.clear();
}

auto ProtocolWithRequests::storeRequestCallback(uint64_t request_id, const callback_t& callback) -> void {
    pending_requests_callbacks_.emplace(request_id, callback);
}

auto ProtocolWithRequests::retrieveRequestCallback(uint64_t request_id) -> std::optional<ProtocolWithRequests::callback_t> {
    auto it = pending_requests_callbacks_.find(request_id);
    if (it != pending_requests_callbacks_.end()) {
        return it->second;
    }
    return std::nullopt;
}

auto ProtocolWithRequests::removeRequestCallback(uint64_t request_id) -> void { pending_requests_callbacks_.erase(request_id); }

auto ProtocolWithRequests::accessPendingRequestsCallbacks() -> std::unordered_map<uint64_t, ProtocolWithRequests::callback_t>& {
    return pending_requests_callbacks_;
}

} // namespace oink_judge::socket