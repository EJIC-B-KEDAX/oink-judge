#include <iostream>
#include <oink_judge/config/config.h>
#include <oink_judge/config/server_config_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/socket/boost_io_context.h>
#include <oink_judge/socket/connection_protocol.h>

using namespace oink_judge;

using config::Config;
using config::getConnectionConfig;
using logger::requireHasValue;

auto main(int argc, char* argv[]) -> int {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << '\n'; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT

    config::ConnectionConfig connection_config = requireHasValue(getConnectionConfig("dispacter"));

    std::cout << "Starting test node and connecting to dispatcher at " << connection_config.host << ":" << connection_config.port
              << '\n';

    co_spawn(oink_judge::socket::BoostIOContext::instance(),
             oink_judge::socket::asyncConnectToTheEndpoint(connection_config.host, connection_config.port,
                                                           connection_config.session_type, connection_config.start_message),
             boost::asio::detached);

    oink_judge::socket::BoostIOContext::instance().run();
}
