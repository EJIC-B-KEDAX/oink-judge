#include "oink_judge/dispatcher/queue_manager_service.h"

#include "oink_judge/dispatcher/invoker.h"
#include "oink_judge/dispatcher/testing_queue.h"
#include "oink_judge/test_node.pb.h"

#include <oink_judge/logger/logger.h>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/use_awaitable.hpp>

namespace oink_judge::dispatcher {

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto readLoop(ConnectRPC& rpc, std::string node_id) -> awaitable<void> {
    while (true) {
        ClientMessage client_message;
        auto res = co_await rpc.read(client_message);
        if (!res) {
            logger::logError("queue_manager_service", "Failed to read client message from " + node_id);
            co_return;
        }
        if (client_message.has_test_submission_response()) {
            TestingQueue::instance().freeInvoker(node_id);
        }
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto writeLoop(ConnectRPC& rpc, std::shared_ptr<channel_t> channel) -> awaitable<void> {
    while (true) {
        auto [ec, submission_id] = co_await channel->async_receive(boost::asio::as_tuple(boost::asio::use_awaitable));
        if (ec) {
            logger::logError("queue_manager_service", "Failed to receive submission from channel");
            co_return;
        }
        ServerMessage server_message;
        server_message.mutable_test_submission_request()->set_submission_id(submission_id);
        co_await rpc.write(server_message, boost::asio::use_awaitable);
        logger::logInfo("queue_manager_service", "Sent submission " + submission_id + " for testing");
    }
}

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
auto connectHandler(ConnectRPC& rpc) -> awaitable<void> {
    ClientMessage handshake_message;
    co_await rpc.read(handshake_message);
    if (!handshake_message.has_handshake()) {
        co_await rpc.finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Handshake message is required"));
        co_return;
    }
    const Handshake& handshake = handshake_message.handshake();
    [[maybe_unused]] std::string node_id = handshake.node_id();
    [[maybe_unused]] std::string node_type = handshake.node_type();

    std::shared_ptr<channel_t> channel = std::make_shared<channel_t>(co_await boost::asio::this_coro::executor, 2);
    // TODO make capacity configurable
    auto invoker = std::make_unique<Invoker>(node_id, channel);
    TestingQueue::instance().connectInvoker(std::move(invoker));

    auto executor = co_await boost::asio::this_coro::executor;

    co_await boost::asio::experimental::parallel_group(
        boost::asio::co_spawn(executor, readLoop(rpc, node_id), boost::asio::deferred),
        boost::asio::co_spawn(executor, writeLoop(rpc, channel), boost::asio::deferred))
        .async_wait(boost::asio::experimental::wait_for_one(), boost::asio::use_awaitable);

    TestingQueue::instance().disconnectInvoker(node_id);
    co_await rpc.finish(grpc::Status::OK);
}

} // namespace oink_judge::dispatcher
