#include "socket/AsyncServer.h"
#include "config/Config.h"
#include "socket/BoostIOContext.h"
#include <iostream>

using Config = oink_judge::config::Config;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << std::endl;
        return 1;
    }

    Config::set_config_file_path(argv[1]);
    Config::set_credential_file_path(argv[2]);


    std::cout << "Starting server on port " << Config::config().at("my_port") << std::endl;
    
    auto server = std::make_shared<oink_judge::socket::AsyncServer>(
        Config::config().at("my_port"), 
        oink_judge::socket::ConnectionHandlerFactory::instance().create(
        Config::config().at("connection_handler_type")));

    server->start_accept();
    oink_judge::socket::BoostIOContext::instance().run();
}
