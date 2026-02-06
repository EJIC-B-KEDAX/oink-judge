#include <iostream>
#include <oink_judge/config/config.h>
#include <oink_judge/socket/boost_io_context.h>
#include <oink_judge/socket/connection_protocol.h>

using Config = oink_judge::config::Config;

auto main(int argc, char* argv[]) -> int {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << '\n'; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT

    std::cout << "Starting test node and connecting to dispatcher at " << Config::config().at("hosts").at("dispatcher") << ":"
              << Config::config().at("ports").at("dispatcher") << '\n';

    std::string host = Config::config().at("hosts").at("dispatcher").get<std::string>();
    short port = Config::config().at("ports").at("dispatcher").get<short>();
    std::string session_type = Config::config().at("sessions").at("dispatcher").get<std::string>();
    std::string start_message = Config::config().at("start_messages").at("dispatcher").get<std::string>();

    std::cerr << "1Start message: " << start_message << '\n';

    co_spawn(oink_judge::socket::BoostIOContext::instance(),
             oink_judge::socket::asyncConnectToTheEndpoint(host, port, session_type, start_message), boost::asio::detached);

    oink_judge::socket::BoostIOContext::instance().run();
}
