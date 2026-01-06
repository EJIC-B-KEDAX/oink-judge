#include "socket/connection_protocol.h"
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


    std::cout << "Starting test node and connecting to dispatcher at "
              << Config::config().at("hosts").at("dispatcher") << ":"
              << Config::config().at("ports").at("dispatcher") << std::endl;

    std::string host = Config::config().at("hosts").at("dispatcher").get<std::string>();
    short port = Config::config().at("ports").at("dispatcher").get<short>();
    std::string session_type = Config::config().at("sessions").at("dispatcher").get<std::string>();
    oink_judge::socket::async_connect_to_the_endpoint(host, port, session_type,
        Config::config().at("start_messages").at("dispatcher").get<std::string>());

    oink_judge::socket::BoostIOContext::instance().run();
}
