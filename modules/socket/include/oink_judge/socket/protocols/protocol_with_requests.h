#pragma once
#include "oink_judge/socket/protocols/protocol_base.h"

namespace oink_judge::socket {

class ProtocolWithRequests : public ProtocolBase {
  public:
    void requestInternal(const std::string& message, const callback_t& callback) override;

    void closeSession() override;

  protected:
    auto storeRequestCallback(uint64_t request_id, const callback_t& callback) -> void;
    auto retrieveRequestCallback(uint64_t request_id) -> std::optional<callback_t>;
    auto removeRequestCallback(uint64_t request_id) -> void;

    auto accessPendingRequestsCallbacks() -> std::unordered_map<uint64_t, callback_t>&;

  private:
    std::unordered_map<uint64_t, callback_t> pending_requests_callbacks_;
};

} // namespace oink_judge::socket
