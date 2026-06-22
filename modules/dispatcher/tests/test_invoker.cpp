#include "oink_judge/dispatcher/invoker.h"

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

TEST(InvokerTest, NullChannelThrows) {
    boost::asio::io_context io(1);
    EXPECT_THROW((void)Invoker("invoker-1", nullptr), std::invalid_argument);
}

TEST(InvokerTest, GetIdReturnsConstructorValue) {
    boost::asio::io_context io(1);
    auto channel = std::make_shared<channel_t>(io.get_executor(), 2);
    Invoker invoker("invoker-42", channel);
    EXPECT_EQ(invoker.getId(), "invoker-42");
}

TEST(InvokerTest, CanTestSubmissionReturnsTrueByDefault) {
    boost::asio::io_context io(1);
    auto channel = std::make_shared<channel_t>(io.get_executor(), 2);
    Invoker invoker("invoker-1", channel);
    EXPECT_TRUE(invoker.canTestSubmission("submission-1"));
}

TEST(InvokerTest, TestSubmissionSendsToChannel) {
    boost::asio::io_context io(1);
    auto channel = std::make_shared<channel_t>(io.get_executor(), 2);
    Invoker invoker("invoker-1", channel);

    invoker.testSubmission("submission-99");

    auto received = receiveSubmission(io, *channel);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, "submission-99");
}

TEST(InvokerTest, CanTestSubmissionCanBeOverridden) {
    boost::asio::io_context io(1);
    auto channel = std::make_shared<channel_t>(io.get_executor(), 2);
    SelectiveInvoker invoker("invoker-1", channel, false);
    EXPECT_FALSE(invoker.canTestSubmission("submission-1"));
}
