#include "oink_judge/content_service/content_server_protocol.h"

#include "oink_judge/content_service/manifest_storage.h"

#include <oink_judge/config/config.h>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

namespace oink_judge::content_service {

using json = nlohmann::json;
using Config = config::Config;
namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().registerType(ContentServerProtocol::REGISTERED_NAME,
                                                     [](const std::string& params) -> std::unique_ptr<ContentServerProtocol> {
                                                         return std::make_unique<ContentServerProtocol>();
                                                     });

    return true;
}();

} // namespace

ContentServerProtocol::ContentServerProtocol() = default;

auto ContentServerProtocol::start(std::string start_message) -> awaitable<void> {
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ContentServerProtocol::receiveMessage(std::string message) -> awaitable<void> {
    json received_json = json::parse(message);

    if (received_json["request"] == "get_manifest") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];

        json manifest = ManifestStorage::instance().getManifest(content_type, content_id).toJson();

        json response_json = {{"__id__", received_json["__id__"]}, {"request", "response"},
                              {"sended_request", "get_manifest"},  {"content_type", content_type},
                              {content_type + "_id", content_id},  {"manifest", manifest}};

        co_spawn(co_await boost::asio::this_coro::executor, sendMessage(response_json.dump()), boost::asio::detached);
    } else if (received_json["request"] == "get_file") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];
        std::string file_path = received_json["file_path"];

        std::string base_directory = Config::config().at("directories").at(content_type + "s").get<std::string>();
        std::string full_file_path = base_directory + "/" + content_id + "/" + file_path;

        std::string file_content = utils::filesystem::loadFile(full_file_path);

        json response_json = {{"__id__", received_json["__id__"]},
                              {"request", "response"},
                              {"sended_request", "get_file"},
                              {"content_type", content_type},
                              {content_type + "_id", content_id},
                              {"file_path", file_path},
                              {"file_content", utils::crypto::toBase64(file_content)}};

        co_spawn(co_await boost::asio::this_coro::executor, sendMessage(response_json.dump()), boost::asio::detached);
    } else if (received_json["request"] == "update_file") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];
        std::string file_path = received_json["file_path"];
        std::string file_content_base64 = received_json["file_content"];

        std::string base_directory = Config::config().at("directories").at(content_type + "s").get<std::string>();
        std::string full_file_path = base_directory + "/" + content_id + "/" + file_path;

        std::string file_content = utils::crypto::fromBase64(file_content_base64);
        utils::filesystem::storeFile(full_file_path, file_content);

        json response_json = {{"__id__", received_json["__id__"]},
                              {"request", "response"},
                              {"sended_request", "update_file"},
                              {"content_type", content_type},
                              {content_type + "_id", content_id},
                              {"file_path", file_path},
                              {"status", "success"}};

        co_spawn(co_await boost::asio::this_coro::executor, sendMessage(response_json.dump()), boost::asio::detached);
    } else if (received_json["request"] == "remove_file") {
        std::string content_type = received_json["content_type"];
        std::string content_id = received_json[content_type + "_id"];
        std::string file_path = received_json["file_path"];

        std::string base_directory = Config::config().at("directories").at(content_type + "s").get<std::string>();
        std::string full_file_path = base_directory + "/" + content_id + "/" + file_path;

        utils::filesystem::removeFileOrDirectory(full_file_path);

        json response_json = {{"__id__", received_json["__id__"]},
                              {"request", "response"},
                              {"sended_request", "remove_file"},
                              {"content_type", content_type},
                              {content_type + "_id", content_id},
                              {"file_path", file_path},
                              {"status", "success"}};

        co_spawn(co_await boost::asio::this_coro::executor, sendMessage(response_json.dump()), boost::asio::detached);
    }

    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ContentServerProtocol::closeSession() -> void {}

} // namespace oink_judge::content_service
