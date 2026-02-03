#include "socket/protocols/ProtocolWithRequests.h"
#include <nlohmann/json.hpp>

namespace oink_judge::socket {

using json = nlohmann::json;

void ProtocolWithRequests::request_internal(const std::string &message, const callback_t &callback) {
    static uint64_t request_id_counter = 0;
    uint64_t request_id = ++request_id_counter;

    store_request_callback(request_id, callback);

    json request_json;

    try {
        request_json = json::parse(message);
    } catch (const json::parse_error &e) {
        request_json = json::object();
        request_json["data"] = message;
    }
    
    request_json["__id__"] = request_id;

    boost::asio::co_spawn(get_session()->get_executor(), send_message(request_json.dump()), boost::asio::detached);
}

void ProtocolWithRequests::close_session() {
    for (auto &[request_id, callback] : _pending_requests_callbacks) {
        call_callback(callback, std::make_error_code(std::errc::operation_canceled));
    }
    _pending_requests_callbacks.clear();
}

void ProtocolWithRequests::store_request_callback(uint64_t request_id, const callback_t &callback) {
    _pending_requests_callbacks.emplace(request_id, callback);
}

std::optional<ProtocolWithRequests::callback_t> ProtocolWithRequests::retrieve_request_callback(uint64_t request_id) {
    auto it = _pending_requests_callbacks.find(request_id);
    if (it != _pending_requests_callbacks.end()) {
        return it->second;
    }
    return std::nullopt;
}

void ProtocolWithRequests::remove_request_callback(uint64_t request_id) {
    _pending_requests_callbacks.erase(request_id);
}

std::unordered_map<uint64_t, ProtocolWithRequests::callback_t> &ProtocolWithRequests::access_pending_requests_callbacks() {
    return _pending_requests_callbacks;
}

} // namespace oink_judge::socket