#pragma once
#include <system_error>

namespace oink_judge::services::data_sender {

enum class network_error_code {
    OK = 0,
    NOT_CONNECTED = 1,
    WRONG_SESSION_EVENT_HANDLER = 2,
    SEND_FAILED = 3,
    RECEIVE_FAILED = 4,
};

// ДАЙ ДЕНЕГ

class network_category : public std::error_category {
public:
    const char* name() const noexcept override;

    std::string message(int ev) const override;
};

std::error_category &get_network_category();

} // namespace oink_judge::services::data_sender
