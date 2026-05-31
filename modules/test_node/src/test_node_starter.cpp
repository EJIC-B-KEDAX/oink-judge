#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/config.h>
#include <oink_judge/config/logger_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/plugin_manager/config_utils.h>
#include <oink_judge/plugin_manager/plugin_manager.h>
#include <oink_judge/socket/client_config_utils.h>
#include <oink_judge/socket/connection_protocol.h>

#include <boost/asio/io_context.hpp>

#include <iostream>

using namespace oink_judge;

using config::Config;
using config::requireHasValue;
using socket::getConnectionConfig;

auto main(int argc, char* argv[]) -> int {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << '\n'; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT

    config::configureLogger(requireHasValue(config::getLoggerConfig()));

    plugin_manager::PluginManager plugin_manager;
    logger::logInfo("test_node_starter", "Loading plugins...");
    for (const auto& plugin_path : plugin_manager::getAllPluginPaths()) {
        if (!plugin_manager.load(plugin_path)) {
            logger::logError("test_node_starter", "Failed to load plugin: " + plugin_path.string());
        } else {
            logger::logSuccess("test_node_starter", "Successfully loaded plugin: " + plugin_path.string());
        }
    }
    logger::logInfo("test_node_starter", "Finished loading plugins");

    socket::ConnectionConfig connection_config = requireHasValue(getConnectionConfig("dispatcher"));

    logger::logInfo("test_node_starter", "Starting test node and connecting to dispatcher at " + connection_config.host + ":" +
                                             std::to_string(connection_config.port));

    boost::asio::io_context io_context;

    co_spawn(io_context,
             oink_judge::socket::asyncConnectToTheEndpoint(connection_config.host, connection_config.port,
                                                           connection_config.session_type, connection_config.start_message),
             boost::asio::detached);

    io_context.run();
}
