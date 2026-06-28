#pragma once
#include "oink_judge/content_service.grpc.pb.h"

#include <agrpc/server_rpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <grpcpp/grpcpp.h>

namespace oink_judge::content_service {

using boost::asio::awaitable;

using GetManifestRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestGetManifest>;
using SetPermissionsRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestSetPermissions>;
using GetFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestGetFile>;
using CreateFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestCreateFile>;
using UpdateFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestUpdateFile>;
using DeleteFileRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestDeleteFile>;
using CreateContentRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestCreateContent>;
using ListContentRPC = agrpc::ServerRPC<&ContentService::AsyncService::RequestListContent>;

auto getManifestHandler(GetManifestRPC& rpc, GetManifestRequest& request) -> awaitable<void>;

auto setPermissionsHandler(SetPermissionsRPC& rpc, SetPermissionsRequest& request) -> awaitable<void>;

auto getFileHandler(GetFileRPC& rpc, GetFileRequest& request) -> awaitable<void>;

auto createFileHandler(CreateFileRPC& rpc) -> awaitable<void>;

auto updateFileHandler(UpdateFileRPC& rpc) -> awaitable<void>;

auto deleteFileHandler(DeleteFileRPC& rpc, DeleteFileRequest& request) -> awaitable<void>;

auto createContentHandler(CreateContentRPC& rpc, CreateContentRequest& request) -> awaitable<void>;

auto listContentHandler(ListContentRPC& rpc, ListContentRequest& request) -> awaitable<void>;

} // namespace oink_judge::content_service
