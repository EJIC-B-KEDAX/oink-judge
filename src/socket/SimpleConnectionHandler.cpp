#include "socket/SimpleConnectionHandler.h"
#include "socket/Session.hpp"
#include "config/Config.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    BasicConnectionHandlerFactory::instance().register_type(SimpleConnectionHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::shared_ptr<SimpleConnectionHandler> {
        return std::make_shared<SimpleConnectionHandler>();
    });

    return true;
}();

} // namespace

using Config = config::Config;
using json = nlohmann::json;

SimpleConnectionHandler::SimpleConnectionHandler() = default;

void SimpleConnectionHandler::new_connection(tcp::socket &socket, const std::string &start_message) {
    json parsed_message = json::parse(start_message);
    auto session = socket::BasicSessionFactory::instance().create(Config::config().at("sessions").at(parsed_message.at("connection_type")).get<std::string>(),
        std::move(socket));

    session->start(start_message);
}

} // namespace oink_judge::services::data_sender
