#pragma once
#include <oink_judge/test_node.grpc.pb.h>

#include <agrpc/server_rpc.hpp>
#include <boost/asio/awaitable.hpp>

namespace oink_judge::dispatcher {

using ServerMessage = oink_judge::test_node::ServerMessage;
using ClientMessage = oink_judge::test_node::ClientMessage;
using Handshake = oink_judge::test_node::Handshake;
using TestSubmissionRequest = oink_judge::test_node::TestSubmissionRequest;
using TestSubmissionResponse = oink_judge::test_node::TestSubmissionResponse;
using QueueManagerService = oink_judge::test_node::QueueManagerService;

using ConnectRPC = agrpc::ServerRPC<&QueueManagerService::AsyncService::RequestConnect>;

using boost::asio::awaitable;

auto connectHandler(ConnectRPC& rpc) -> awaitable<void>;

} // namespace oink_judge::dispatcher
