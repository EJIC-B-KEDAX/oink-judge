#include "socket/AsyncServer.h"
#include "config/Config.h"
#include "socket/BoostIOContext.h"

using Config = oink_judge::config::Config;

int main() {
    oink_judge::socket::AsyncServer server(
        Config::config().at("my_port"), 
        oink_judge::socket::BasicConnectionHandlerFactory::instance().create(
        Config::config().at("connection_handler_type")));

    server.start_accept();
    oink_judge::socket::BoostIOContext::instance().run();
}
