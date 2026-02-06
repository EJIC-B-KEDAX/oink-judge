#include <iostream>
#include <oink_judge/config/config.h>
#include <oink_judge/socket/async_server.h>
#include <oink_judge/socket/boost_io_context.h>

using Config = oink_judge::config::Config;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << '\n'; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT

    std::cout << "Starting server on port " << Config::config().at("my_port") << '\n';

    auto server = std::make_shared<oink_judge::socket::AsyncServer>(
        Config::config().at("my_port"),
        oink_judge::socket::ConnectionHandlerFactory::instance().create(Config::config().at("connection_handler_type")));

    server->startAccept();
    oink_judge::socket::BoostIOContext::instance().run();

    std::cerr << "Server stopped." << '\n';
}
