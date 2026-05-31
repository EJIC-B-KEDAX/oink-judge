#include "oink_judge/socket/simple_connection_handler.h"

#include "oink_judge/socket/client_config_utils.h"
#include "oink_judge/socket/session.hpp"

#include <oink_judge/config/common_utils.h>

#include <nlohmann/json.hpp>

namespace oink_judge::socket {

using config::requireHasValue;

using json = nlohmann::json;

SimpleConnectionHandler::SimpleConnectionHandler() = default;

auto SimpleConnectionHandler::newConnection(tcp::socket socket, std::string start_message) -> awaitable<void> {
    json parsed_message = json::parse(start_message);
    std::string connection_type = parsed_message.at("connection_type");
    auto session = socket::SessionFactory::instance().create(requireHasValue(getSessionType(connection_type)), std::move(socket));

    co_await session->start(start_message);
}

auto registerSimpleConnectionHandlerType() -> void {
    ConnectionHandlerFactory::instance().registerType(SimpleConnectionHandler::REGISTERED_NAME,
                                                      [](const std::string& params) -> std::shared_ptr<SimpleConnectionHandler> {
                                                          return std::make_shared<SimpleConnectionHandler>();
                                                      });
}

} // namespace oink_judge::socket
