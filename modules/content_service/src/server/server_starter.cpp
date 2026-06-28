#include "oink_judge/content_service/server/content_service.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/config.h>
#include <oink_judge/config/logger_utils.h>
#include <oink_judge/config/server_utils.h>
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

using content_service::ContentService;
using content_service::CreateContentRPC;
using content_service::CreateFileRPC;
using content_service::DeleteFileRPC;
using content_service::GetFileRPC;
using content_service::GetManifestRPC;
using content_service::ListContentRPC;
using content_service::SetPermissionsRPC;
using content_service::UpdateFileRPC;

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

    ContentService::AsyncService service;
    grpc::ServerBuilder builder;
    auto server_credentials = requireHasValue(utils::grpc::getServerCredentials());
    builder.AddListeningPort(server_address, server_credentials);
    builder.RegisterService(&service);

    auto factories = utils::grpc::getInterceptorsForServer();
    builder.experimental().SetInterceptorCreators(std::move(factories));

    agrpc::GrpcContext grpc_ctx{builder.AddCompletionQueue()};
    auto server = builder.BuildAndStart();

    agrpc::register_awaitable_rpc_handler<GetManifestRPC>(grpc_ctx, service, &content_service::getManifestHandler,
                                                          boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<SetPermissionsRPC>(grpc_ctx, service, &content_service::setPermissionsHandler,
                                                             boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<GetFileRPC>(grpc_ctx, service, &content_service::getFileHandler, boost::asio::detached);

    agrpc::register_awaitable_rpc_handler<CreateFileRPC>(grpc_ctx, service, &content_service::createFileHandler,
                                                         boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<UpdateFileRPC>(grpc_ctx, service, &content_service::updateFileHandler,
                                                         boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<DeleteFileRPC>(grpc_ctx, service, &content_service::deleteFileHandler,
                                                         boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<CreateContentRPC>(grpc_ctx, service, &content_service::createContentHandler,
                                                            boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<ListContentRPC>(grpc_ctx, service, &content_service::listContentHandler,
                                                          boost::asio::detached);

    grpc_ctx.run();

    logger::logInfo("server_starter", "Server stopped");
}
