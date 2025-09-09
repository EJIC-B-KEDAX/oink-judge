#include "socket/connection_protocol.h"
#include "config/Config.h"
#include "socket/BoostIOContext.h"

using Config = oink_judge::config::Config;

int main() {
    std::string host = Config::config().at("hosts").at("dispatcher").get<std::string>();
    short port = Config::config().at("ports").at("dispatcher").get<short>();
    std::string session_type = Config::config().at("sessions").at("dispatcher").get<std::string>();
    oink_judge::socket::async_connect_to_the_endpoint(host, port, session_type,
        Config::config().at("start_messages").at("dispatcher").get<std::string>());

    oink_judge::socket::BoostIOContext::instance().run();
}
