#include "services/data_sender/data_sender_errors.h"

namespace oink_judge::services::data_sender {

const char* network_category::name() const noexcept {
    return "data_sender_network";
}

std::string network_category::message(int ev) const {
    switch (static_cast<network_error_code>(ev)) {
        case network_error_code::OK:
            return "No error";
        case network_error_code::NOT_CONNECTED:
            return "Not connected to the server";
        case network_error_code::WRONG_SESSION_EVENT_HANDLER:
            return "Wrong session event handler type";
        case network_error_code::SEND_FAILED:
            return "Failed to send message";
        case network_error_code::RECEIVE_FAILED:
            return "Failed to receive message";
        default:
            return "Unknown error";
    }
}

std::error_category &get_network_category() {
    static network_category instance;
    return instance;
}

} // namespace oink_judge::services::data_sender
