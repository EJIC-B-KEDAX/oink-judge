#include "oink_judge/dispatcher/protocol_with_invoker.h"

#include "oink_judge/dispatcher/testing_queue.h"

#include <nlohmann/json.hpp>

namespace oink_judge::dispatcher {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().registerType(ProtocolWithInvoker::REGISTERED_NAME,
                                                     [](const std::string& params) -> std::unique_ptr<ProtocolWithInvoker> {
                                                         return std::make_unique<ProtocolWithInvoker>();
                                                     });

    return true;
}();

} // namespace

using json = nlohmann::json;

ProtocolWithInvoker::ProtocolWithInvoker() = default;

auto ProtocolWithInvoker::start(std::string start_message) -> awaitable<void> {
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ProtocolWithInvoker::receiveMessage(std::string message) -> awaitable<void> {
    if (invoker_id_.empty()) {
        json response = json::parse(message);
        invoker_id_ = response["invoker_id"];

        if (invoker_id_.empty()) {
            co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
            co_return;
        }

        auto invoker = std::make_unique<Invoker>(invoker_id_, getSession());
        TestingQueue::instance().connectInvoker(std::move(invoker));
    } else {
        if (message == "I am free") {
            TestingQueue::instance().freeInvoker(invoker_id_);
        }
    }

    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

void ProtocolWithInvoker::closeSession() {
    if (invoker_id_.empty()) {
        return;
    }

    TestingQueue::instance().disconnectInvoker(invoker_id_);
    invoker_id_.clear();
}

} // namespace oink_judge::dispatcher
