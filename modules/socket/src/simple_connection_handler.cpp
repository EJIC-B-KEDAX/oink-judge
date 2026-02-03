#include "oink_judge/socket/simple_connection_handler.h"

#include "config/Config.h"
#include "oink_judge/socket/session.hpp"

#include <iostream>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    ConnectionHandlerFactory::instance().register_type(SimpleConnectionHandler::REGISTERED_NAME,
                                                       [](const std::string& params) -> std::shared_ptr<SimpleConnectionHandler> {
                                                           return std::make_shared<SimpleConnectionHandler>();
                                                       });

    return true;
}();

} // namespace

using Config = config::Config;
using json = nlohmann::json;

SimpleConnectionHandler::SimpleConnectionHandler() = default;

awaitable<void> SimpleConnectionHandler::new_connection(tcp::socket& socket, const std::string& start_message) {
    json parsed_message = json::parse(start_message);
    std::cout << "New connection of type " << parsed_message.at("connection_type").get<std::string>() << std::endl;
    auto session = socket::SessionFactory::instance().create(
        Config::config().at("sessions").at(parsed_message.at("connection_type")).get<std::string>(), std::move(socket));

    std::cout << "Starting session" << std::endl;

    co_await session->start(start_message);
}

} // namespace oink_judge::socket
