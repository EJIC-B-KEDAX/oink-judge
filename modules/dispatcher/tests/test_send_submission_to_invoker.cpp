#include "oink_judge/dispatcher/invoker.h"
#include "oink_judge/dispatcher/send_submission_to_invoker.h"
#include "oink_judge/dispatcher/testing_queue.h"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>

using boost::asio::awaitable;
using namespace oink_judge::dispatcher;

namespace {

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

class SendSubmissionToInvokerTest : public ::testing::Test {
  protected:
    auto TearDown() -> void override {
        if (invoker_connected_) {
            TestingQueue::instance().freeInvoker("invoker-1");
            TestingQueue::instance().disconnectInvoker("invoker-1");
            invoker_connected_ = false;
        }
    }

    auto connectAcceptingInvoker() -> void {
        channel_ = std::make_shared<channel_t>(io_.get_executor(), 4);
        TestingQueue::instance().connectInvoker(std::make_unique<Invoker>("invoker-1", channel_));
        invoker_connected_ = true;
    }

    auto receiveFromInvoker() -> std::optional<std::string> { return receiveSubmission(io_, *channel_); }

  private:
    boost::asio::io_context io_{1};
    std::shared_ptr<channel_t> channel_;
    bool invoker_connected_ = false;
};

TEST_F(SendSubmissionToInvokerTest, HandleSubmissionPushesToTestingQueue) {
    connectAcceptingInvoker();

    SendSubmissionToInvoker manager("problem-1");
    manager.handleSubmission("submission-42");

    auto received = receiveFromInvoker();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "submission-42");
}

TEST(SendSubmissionToInvokerFactoryTest, RegisteredTypeCreatesManager) {
    registerSendSubmissionToInvokerType();

    auto manager = ProblemSubmissionManagerFactory::instance().create(SendSubmissionToInvoker::REGISTERED_NAME, "problem-1");

    ASSERT_NE(manager, nullptr);
    EXPECT_NE(dynamic_cast<SendSubmissionToInvoker*>(manager.get()), nullptr);
}
