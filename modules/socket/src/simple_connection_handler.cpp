#include "oink_judge/socket/simple_connection_handler.h"

#include "oink_judge/socket/session.hpp"

#include <oink_judge/config/config.h>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ConnectionHandlerFactory::instance().registerType(SimpleConnectionHandler::REGISTERED_NAME,
                                                      [](const std::string& params) -> std::shared_ptr<SimpleConnectionHandler> {
                                                          return std::make_shared<SimpleConnectionHandler>();
                                                      });

    return true;
}();

} // namespace

using Config = config::Config;
using json = nlohmann::json;

SimpleConnectionHandler::SimpleConnectionHandler() = default;

auto SimpleConnectionHandler::newConnection(tcp::socket socket, std::string start_message) -> awaitable<void> {
    json parsed_message = json::parse(start_message);
    auto session = socket::SessionFactory::instance().create(
        Config::config().at("sessions").at(parsed_message.at("connection_type")).get<std::string>(), std::move(socket));

    co_await session->start(start_message);
}

} // namespace oink_judge::socket
