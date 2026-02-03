#pragma once
#include "socket/protocols/ProtocolBase.h"

namespace oink_judge::socket {

class ProtocolWithRequests : public ProtocolBase {
public:
    void request_internal(const std::string &message, const callback_t &callback) override;

    void close_session() override;

protected:
    void store_request_callback(uint64_t request_id, const callback_t &callback);
    std::optional<callback_t> retrieve_request_callback(uint64_t request_id);
    void remove_request_callback(uint64_t request_id);

    std::unordered_map<uint64_t, callback_t> &access_pending_requests_callbacks();

private:
    std::unordered_map<uint64_t, callback_t> _pending_requests_callbacks;
};

} // namespace oink_judge::socket
