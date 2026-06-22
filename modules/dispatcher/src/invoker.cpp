#include "oink_judge/dispatcher/invoker.h"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

namespace oink_judge::dispatcher {

Invoker::Invoker(std::string id, std::shared_ptr<channel_t> channel) : id_(std::move(id)), channel_(std::move(channel)) {
    if (!channel_) {
        throw std::invalid_argument("Channel cannot be null");
    }
}

auto Invoker::getId() const -> const std::string& { return id_; }

auto Invoker::testSubmission(const std::string& submission_id) -> void {
    boost::asio::co_spawn(channel_->get_executor(),
                          channel_->async_send(boost::system::error_code(), submission_id, boost::asio::use_awaitable),
                          boost::asio::detached);
}

auto Invoker::canTestSubmission(const std::string& submission_id) const -> bool {
    // Default implementation, can be overridden by derived classes
    return true; // Assuming all invokers can test submissions by default
}

} // namespace oink_judge::dispatcher
