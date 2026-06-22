#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/config.h>
#include <oink_judge/config/logger_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/plugin_manager/config_utils.h>
#include <oink_judge/plugin_manager/plugin_manager.h>
#include <oink_judge/test_node/config_utils.h>
#include <oink_judge/test_node/queue_manager_service_stub.h>

#include <agrpc/grpc_context.hpp>
#include <agrpc/grpc_executor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>

#include <iostream>

using namespace oink_judge;

using config::Config;
using config::requireHasValue;

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

    auto queue_manager_service_stub_type = requireHasValue(test_node::getQueueManagerServiceStubType());

    auto queue_manager_service_stub =
        test_node::QueueManagerServiceStubFactory::instance().create(queue_manager_service_stub_type);

    agrpc::GrpcContext grpc_ctx;

    std::string test_node_id = requireHasValue(test_node::getMyTestNodeId());
    std::string test_node_type = requireHasValue(test_node::getMyTestNodeType());

    logger::logInfo("test_node_starter", "Connecting to queue manager as " + test_node_id);
    boost::asio::co_spawn(grpc_ctx, queue_manager_service_stub->connect(test_node_id, test_node_type), boost::asio::detached);

    grpc_ctx.run();
    logger::logInfo("test_node_starter", "Disconnected from queue manager");
}
