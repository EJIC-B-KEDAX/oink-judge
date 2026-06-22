#include "oink_judge/dispatcher/dispatcher_service.h"
#include "oink_judge/dispatcher/queue_manager_service.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/config.h>
#include <oink_judge/config/logger_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/plugin_manager/config_utils.h>
#include <oink_judge/plugin_manager/plugin_manager.h>
#include <oink_judge/utils/grpc/config_utils.h>

#include <agrpc/register_awaitable_rpc_handler.hpp>
#include <boost/asio.hpp>
#include <grpcpp/server_builder.h>

#include <iostream>

using namespace oink_judge;

using config::Config;
using config::requireHasValue;

using dispatcher::ConnectRPC;
using dispatcher::DispatcherService;
using dispatcher::HandleSubmissionRPC;
using dispatcher::QueueManagerService;

auto main(int argc, char* argv[]) -> int {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credential_file_path>" << '\n'; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT

    config::configureLogger(requireHasValue(config::getLoggerConfig()));

    std::string server_address = requireHasValue(utils::grpc::getMyEndpoint());

    logger::logInfo("server_starter", "Starting server on " + server_address);

    plugin_manager::PluginManager plugin_manager;
    logger::logInfo("server_starter", "Loading plugins...");
    for (const auto& plugin_path : plugin_manager::getAllPluginPaths()) {
        if (!plugin_manager.load(plugin_path)) {
            logger::logError("server_starter", "Failed to load plugin: " + plugin_path.string());
        } else {
            logger::logSuccess("server_starter", "Successfully loaded plugin: " + plugin_path.string());
        }
    }
    logger::logInfo("server_starter", "Finished loading plugins");

    DispatcherService::AsyncService dispatcher_service;
    QueueManagerService::AsyncService queue_manager_service;
    grpc::ServerBuilder builder;
    auto server_credentials = requireHasValue(utils::grpc::getServerCredentials());
    builder.AddListeningPort(server_address, server_credentials);
    utils::grpc::applyServerChannelArguments(builder);
    builder.RegisterService(&dispatcher_service);
    builder.RegisterService(&queue_manager_service);

    auto factories = utils::grpc::getInterceptorsForServer();
    builder.experimental().SetInterceptorCreators(std::move(factories));

    agrpc::GrpcContext grpc_ctx{builder.AddCompletionQueue()};
    auto server = builder.BuildAndStart();

    agrpc::register_awaitable_rpc_handler<HandleSubmissionRPC>(grpc_ctx, dispatcher_service, &dispatcher::handleSubmissionHandler,
                                                               boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<ConnectRPC>(grpc_ctx, queue_manager_service, &dispatcher::connectHandler,
                                                      boost::asio::detached);

    grpc_ctx.run();

    logger::logInfo("server_starter", "Server stopped");
}
