#include <iostream>
#include <oink_judge/config/config.h>
#include <oink_judge/config/server_config_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/plugin_manager/config_utils.h>
#include <oink_judge/plugin_manager/plugin_manager.h>
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

    std::cout << "Loading configuration..." << std::endl; // NOLINT

    plugin_manager::PluginManager plugin_manager;
    logger::logInfo("server_starter", "Loading plugins...");
    for (const auto& plugin_path : plugin_manager::getAllPluginPaths()) {
        if (!plugin_manager.load(plugin_path)) {
            logger::logError("server_starter", "Failed to load plugin: " + plugin_path.string());
        } else {
            logger::logSuccess("server_starter", "Successfully loaded plugin: " + plugin_path.string());
        }
    }

    int my_port = requireHasValue(getMyPort());

    logger::logMessage("server_starter", "Starting server on port " + std::to_string(my_port));

    auto server = std::make_shared<oink_judge::socket::AsyncServer>(
        my_port, oink_judge::socket::ConnectionHandlerFactory::instance().create(requireHasValue(getConnectionHandlerType())));

    server->startAccept();
    oink_judge::socket::BoostIOContext::instance().run();

    logger::logMessage("server_starter", "Server stopped");
}
