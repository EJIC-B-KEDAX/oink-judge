#pragma once
#include "oink_judge/test_node.grpc.pb.h"

#include <oink_judge/factory/parameterized_type_factory.hpp>

#include <boost/asio/awaitable.hpp>
#include <grpcpp/grpcpp.h>
#include <tl/expected.hpp>

namespace oink_judge::test_node {

using boost::asio::awaitable;

class QueueManagerServiceStub {
  public:
    QueueManagerServiceStub(std::shared_ptr<grpc::Channel> channel);
    auto connect(std::string test_node_id, std::string test_node_type) -> awaitable<tl::expected<void, grpc::Status>>;

    constexpr static auto REGISTERED_NAME = "queue_manager_service_stub";

  private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<QueueManagerService::Stub> stub_;
};

auto registerQueueManagerServiceStubType() -> void;

using QueueManagerServiceStubFactory = factory::ParameterizedTypeFactory<std::unique_ptr<QueueManagerServiceStub>>;

} // namespace oink_judge::test_node
