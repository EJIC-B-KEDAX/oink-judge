#include "oink_judge/content_service/server/content_service.h"

#include <oink_judge/config/config.h>

#include <agrpc/grpc_context.hpp>
#include <agrpc/register_awaitable_rpc_handler.hpp>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_builder.h>

#include <iostream>
#include <string>

using namespace oink_judge::content_service;
using oink_judge::config::Config;

auto main(int argc, char* argv[]) -> int {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path> <credentials_file_path> <listen_address>\n"; // NOLINT
        return 1;
    }

    Config::setConfigFilePath(argv[1]);      // NOLINT
    Config::setCredentialsFilePath(argv[2]); // NOLINT
    Config::reloadData();

    const std::string listen_address = argv[3]; // NOLINT

    ContentService::AsyncService service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    agrpc::GrpcContext grpc_context{builder.AddCompletionQueue()};
    auto server = builder.BuildAndStart();
    if (server == nullptr) {
        std::cerr << "Failed to start content service test server on " << listen_address << '\n';
        return 1;
    }

    agrpc::register_awaitable_rpc_handler<GetManifestRPC>(grpc_context, service, &getManifestHandler,
                                                          boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<SetPermissionsRPC>(grpc_context, service, &setPermissionsHandler,
                                                             boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<GetFileRPC>(grpc_context, service, &getFileHandler, boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<CreateFileRPC>(grpc_context, service, &createFileHandler,
                                                         boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<UpdateFileRPC>(grpc_context, service, &updateFileHandler,
                                                         boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<DeleteFileRPC>(grpc_context, service, &deleteFileHandler,
                                                         boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<CreateContentRPC>(grpc_context, service, &createContentHandler,
                                                              boost::asio::detached);
    agrpc::register_awaitable_rpc_handler<ListContentRPC>(grpc_context, service, &listContentHandler,
                                                           boost::asio::detached);

    grpc_context.run();
    server->Shutdown();

    return 0;
}
