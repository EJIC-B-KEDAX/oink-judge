#include <iostream>
#include <oink_judge/config/config.h>
#include <oink_judge/config/server_config_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/socket/async_server.h>
#include <oink_judge/socket/boost_io_context.h>

using namespace oink_judge;

using config::Config;
using config::getConnectionHandlerType;
using config::getMyPort;
using logger::requireHasValue;

auto main(int argc, char* argv[]) -> int {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << '\n'; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT

    int my_port = requireHasValue(getMyPort());

    logger::logMessage("server_starter", "Starting server on port " + std::to_string(my_port));

    auto server = std::make_shared<oink_judge::socket::AsyncServer>(
        my_port, oink_judge::socket::ConnectionHandlerFactory::instance().create(requireHasValue(getConnectionHandlerType())));

    server->startAccept();
    oink_judge::socket::BoostIOContext::instance().run();

    logger::logMessage("server_starter", "Server stopped");
}
