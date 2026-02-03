#include "services/dispatcher/ProtocolWithInvoker.h"

#include "services/dispatcher/TestingQueue.h"

#include <nlohmann/json.hpp>

namespace oink_judge::services::dispatcher {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().register_type(ProtocolWithInvoker::REGISTERED_NAME,
                                                      [](const std::string& params) -> std::unique_ptr<ProtocolWithInvoker> {
                                                          return std::make_unique<ProtocolWithInvoker>();
                                                      });

    return true;
}();

} // namespace

using json = nlohmann::json;

ProtocolWithInvoker::ProtocolWithInvoker() = default;

awaitable<void> ProtocolWithInvoker::start(std::string start_message) {
    co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
}

awaitable<void> ProtocolWithInvoker::receive_message(std::string message) {
    if (invoker_id_.empty()) {
        json response = json::parse(message);
        invoker_id_ = response["invoker_id"];

        if (invoker_id_.empty()) {
            co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
            co_return;
        }

        auto invoker = std::make_unique<Invoker>(invoker_id_, get_session());
        TestingQueue::instance().connect_invoker(std::move(invoker));
    } else {
        if (message == "I am free") {
            TestingQueue::instance().free_invoker(invoker_id_);
        }
    }

    co_spawn(co_await boost::asio::this_coro::executor, get_session()->receive_message(), boost::asio::detached);
}

void ProtocolWithInvoker::close_session() {
    if (invoker_id_.empty()) {
        return;
    }

    TestingQueue::instance().disconnect_invoker(invoker_id_);
    invoker_id_.clear();
}

} // namespace oink_judge::services::dispatcher
