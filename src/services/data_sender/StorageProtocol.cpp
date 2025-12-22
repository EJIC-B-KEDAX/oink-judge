#include "services/data_sender/StorageProtocol.h"
#include "config/Config.h"
#include "services/data_sender/zip_utils.h"

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

StorageProtocol::StorageProtocol() : _status(WAIT_HEADER), _now_response_id(0) {};

void StorageProtocol::start(const std::string &start_message) {
    _status = WAIT_HEADER;
    get_session()->receive_message();
}

void StorageProtocol::receive_message(const std::string &message) {
    if (_status == WAIT_HEADER) {
        json header = json::parse(message);

        _content_type = header["content_type"];
        _content_id = header[_content_type + "_id"];
        _now_response_id = header["__id__"];
        _status = WAIT_DATA;
    } else if (_status == WAIT_DATA) {
        clear_directory(Config::config().at("directories").at(_content_type + "s").get<std::string>() + "/" + _content_id);
        store_zip(Config::config().at("directories").at(_content_type + "s_zip").get<std::string>() + "/" + _content_id + ".zip", message);
        unpack_zip(Config::config().at("directories").at(_content_type + "s_zip").get<std::string>() + "/" + _content_id + ".zip",
            Config::config().at("directories").at(_content_type + "s").get<std::string>() + "/" + _content_id);
            
        _status = WAIT_HEADER;

        auto callback_opt = retrieve_request_callback(_now_response_id);
        if (callback_opt) {
            auto callback = *callback_opt;
            call_callback(callback, std::error_code{});
            remove_request_callback(_now_response_id);
        }
    }
    
    get_session()->receive_message();
}

void StorageProtocol::close_session() {
    socket::ProtocolWithRequests::close_session();
    // Reconnect to the server
}

} // namespace oink_judge::services::data_sender
