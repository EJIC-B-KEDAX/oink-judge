#pragma once
#include "oink_judge/content_service.grpc.pb.h"

#include <agrpc/server_rpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <grpcpp/grpcpp.h>

namespace oink_judge::content_service {

using boost::asio::awaitable;

using GetManifestRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestGetManifest>;
using GetFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestGetFile>;
using CreateFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestCreateFile>;
using UpdateFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestUpdateFile>;
using DeleteFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestDeleteFile>;

auto getManifestHandler(GetManifestRPC& rpc, GetManifestRequest& request) -> awaitable<void>;

auto getFileHandler(GetFileRPC& rpc, GetFileRequest& request) -> awaitable<void>;

auto createFileHandler(CreateFileRPC& rpc) -> awaitable<void>;

auto updateFileHandler(UpdateFileRPC& rpc) -> awaitable<void>;

auto deleteFileHandler(DeleteFileRPC& rpc, DeleteFileRequest& request) -> awaitable<void>;

} // namespace oink_judge::content_service
