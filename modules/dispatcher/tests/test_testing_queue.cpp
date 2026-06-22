#include "oink_judge/dispatcher/invoker.h"
#include "oink_judge/dispatcher/testing_queue.h"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using boost::asio::awaitable;
using namespace oink_judge::dispatcher;

namespace {

class SelectiveInvoker : public Invoker {
  public:
    SelectiveInvoker(std::string id, std::shared_ptr<channel_t> channel, bool accepts)
        : Invoker(std::move(id), std::move(channel)), accepts_(accepts) {}

    auto canTestSubmission(const std::string& /*submission_id*/) const -> bool override { return accepts_; }

  private:
    bool accepts_;
};

auto receiveSubmission(boost::asio::io_context& io, channel_t& channel) -> std::optional<std::string> {
    std::optional<std::string> result;

    boost::asio::co_spawn(
        io,
        [&]() -> awaitable<void> { // NOLINT
            auto [ec, submission_id] = co_await channel.async_receive(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (!ec) {
                result = submission_id;
            }
        },
        boost::asio::detached);

    io.run_for(std::chrono::milliseconds(500)); // NOLINT
    if (result.has_value()) {
        io.restart();
    }
    return result;
}

} // namespace

class TestingQueueTest : public ::testing::Test {
  protected:
    auto TearDown() -> void override {
        for (const auto& invoker_id : connected_invokers_) {
            TestingQueue::instance().freeInvoker(invoker_id);
            TestingQueue::instance().disconnectInvoker(invoker_id);
        }
        connected_invokers_.clear();
    }

    auto connectInvoker(const std::string& invoker_id, bool accepts) -> std::shared_ptr<channel_t> {
        auto channel = std::make_shared<channel_t>(io_.get_executor(), 8); // NOLINT
        TestingQueue::instance().connectInvoker(std::make_unique<SelectiveInvoker>(invoker_id, channel, accepts));
        connected_invokers_.push_back(invoker_id);
        return channel;
    }

    auto receiveFrom(const std::shared_ptr<channel_t>& channel) -> std::optional<std::string> {
        return receiveSubmission(io_, *channel);
    }

    auto unregisterInvoker(const std::string& invoker_id) -> void {
        connected_invokers_.erase(std::ranges::remove(connected_invokers_, invoker_id).begin(), connected_invokers_.end());
    }

  private:
    boost::asio::io_context io_{1};
    std::vector<std::string> connected_invokers_;
};

TEST_F(TestingQueueTest, PushSubmissionDispatchesToIdleInvoker) {
    auto channel = connectInvoker("invoker-1", true);

    TestingQueue::instance().pushSubmission("submission-1");

    auto received = receiveFrom(channel);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "submission-1");
}

TEST_F(TestingQueueTest, PushSubmissionQueuesWhenNoInvokerAvailable) {
    connectInvoker("rejector", false);

    TestingQueue::instance().pushSubmission("queued-submission");

    auto acceptor_channel = connectInvoker("acceptor", true);
    auto received = receiveFrom(acceptor_channel);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "queued-submission");
}

TEST_F(TestingQueueTest, ConnectInvokerDispatchesQueuedSubmission) {
    TestingQueue::instance().pushSubmission("queued-submission");

    auto channel = connectInvoker("invoker-1", true);
    auto received = receiveFrom(channel);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "queued-submission");
}

TEST_F(TestingQueueTest, FreeInvokerDispatchesNextQueuedSubmission) {
    auto first_channel = connectInvoker("invoker-1", true);
    TestingQueue::instance().pushSubmission("submission-1");
    ASSERT_TRUE(receiveFrom(first_channel).has_value());

    TestingQueue::instance().pushSubmission("submission-2");

    TestingQueue::instance().freeInvoker("invoker-1");

    auto received = receiveFrom(first_channel);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "submission-2");
}

TEST_F(TestingQueueTest, DisconnectInvokerRequeuesInProgressSubmission) {
    auto first_channel = connectInvoker("invoker-1", true);
    TestingQueue::instance().pushSubmission("in-progress-submission");

    TestingQueue::instance().disconnectInvoker("invoker-1");
    unregisterInvoker("invoker-1");

    auto second_channel = connectInvoker("invoker-2", true);
    auto received = receiveFrom(second_channel);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "in-progress-submission");
}

TEST_F(TestingQueueTest, FreeInvokerThrowsForUnknownInvoker) {
    EXPECT_THROW(TestingQueue::instance().freeInvoker("missing-invoker"), std::runtime_error);
}

TEST_F(TestingQueueTest, DisconnectInvokerThrowsForUnknownInvoker) {
    EXPECT_THROW(TestingQueue::instance().disconnectInvoker("missing-invoker"), std::runtime_error);
}
