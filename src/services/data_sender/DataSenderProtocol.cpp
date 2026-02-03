#include "services/data_sender/DataSenderProtocol.h"
#include "config/Config.h"
#include "services/data_sender/ManifestStorage.h"
#include "utils/filesystem.h"
#include "utils/crypto.h"
#include <iostream>

namespace oink_judge::services::data_sender {

using json = nlohmann::json;
using Config = config::Config;
namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::ProtocolFactory::instance().register_type(DataSenderProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<DataSenderProtocol> {
        return std::make_unique<DataSenderProtocol>();
    });

    return true;
}();

} // namespace

DataSenderProtocol::DataSenderProtocol() {}

awaitable<void> DataSenderProtocol::start(const std::string &start_message) {
    co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
}

awaitable<void> DataSenderProtocol::receive_message(const std::string &message) {
    json received_json = json::parse(message);

    std::cerr << "DataSenderProtocol received message: " << received_json.dump() << std::endl;
    
    if (received_json["request"] == "get_manifest") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];

        std::cerr << "Fetching manifest for " << content_type << " " << content_id << std::endl;

        json manifest = ManifestStorage::instance().get_manifest(content_type, content_id).to_json();

        std::cerr << "Fetched manifest: " << manifest.dump() << std::endl;

        json response_json = {
            {"__id__", received_json["__id__"]},
            {"request", "response"},
            {"sended_request", "get_manifest"},
            {"content_type", content_type},
            {content_type + "_id", content_id},
            {"manifest", manifest}
        };

        std::cerr << "Sending manifest response: " << response_json.dump() << std::endl;

        co_spawn(co_await boost::asio::this_coro::executor, send_message(response_json.dump()), boost::asio::detached);
    } else if (received_json["request"] == "get_file") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];
        std::string file_path = received_json["file_path"];

        std::string base_directory = Config::config().at("directories").at(content_type + "s").get<std::string>();
        std::string full_file_path = base_directory + "/" + content_id + "/" + file_path;

        std::string file_content = utils::filesystem::load_file(full_file_path);

        json response_json = {
            {"__id__", received_json["__id__"]},
            {"request", "response"},
            {"sended_request", "get_file"},
            {"content_type", content_type},
            {content_type + "_id", content_id},
            {"file_path", file_path},
            {"file_content", utils::crypto::to_base64(file_content)}
        };
        
        co_spawn(co_await boost::asio::this_coro::executor, send_message(response_json.dump()), boost::asio::detached);
    } else if (received_json["request"] == "update_file") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];
        std::string file_path = received_json["file_path"];
        std::string file_content_base64 = received_json["file_content"];

        std::string base_directory = Config::config().at("directories").at(content_type + "s").get<std::string>();
        std::string full_file_path = base_directory + "/" + content_id + "/" + file_path;

        std::string file_content = utils::crypto::from_base64(file_content_base64);
        utils::filesystem::store_file(full_file_path, file_content);

        json response_json = {
            {"__id__", received_json["__id__"]},
            {"request", "response"},
            {"sended_request", "update_file"},
            {"content_type", content_type},
            {content_type + "_id", content_id},
            {"file_path", file_path},
            {"status", "success"}
        };

        co_spawn(co_await boost::asio::this_coro::executor, send_message(response_json.dump()), boost::asio::detached);
    } else if (received_json["request"] == "remove_file") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];
        std::string file_path = received_json["file_path"];

        std::string base_directory = Config::config().at("directories").at(content_type + "s").get<std::string>();
        std::string full_file_path = base_directory + "/" + content_id + "/" + file_path;

        utils::filesystem::remove_file_or_directory(full_file_path);

        json response_json = {
            {"__id__", received_json["__id__"]},
            {"request", "response"},
            {"sended_request", "remove_file"},
            {"content_type", content_type},
            {content_type + "_id", content_id},
            {"file_path", file_path},
            {"status", "success"}
        };

        co_spawn(co_await boost::asio::this_coro::executor, send_message(response_json.dump()), boost::asio::detached);
    }

    co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
}

void DataSenderProtocol::close_session() {}

} // namespace oink_judge::services::data_sender
