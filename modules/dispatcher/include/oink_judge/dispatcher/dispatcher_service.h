#pragma once
#include "oink_judge/dispatcher.grpc.pb.h"

#include <agrpc/server_rpc.hpp>
#include <boost/asio/awaitable.hpp>
#include <grpcpp/grpcpp.h>

namespace oink_judge::dispatcher {

using boost::asio::awaitable;

using HandleSubmissionRPC = agrpc::ServerRPC<&DispatcherService::AsyncService::RequestHandleSubmission>;

auto handleSubmissionHandler(HandleSubmissionRPC& rpc, HandleSubmissionRequest& request) -> awaitable<void>;

} // namespace oink_judge::dispatcher