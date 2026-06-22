#pragma once
#include <boost/asio/experimental/channel.hpp>

#include <memory>

namespace oink_judge::dispatcher {

using channel_t = boost::asio::experimental::channel<void(boost::system::error_code, std::string)>;

class Invoker {
  public:
    Invoker(std::string id, std::shared_ptr<channel_t> channel);

    Invoker(const Invoker&) = delete;
    auto operator=(const Invoker&) -> Invoker& = delete;
    Invoker(Invoker&&) = delete;
    auto operator=(Invoker&&) -> Invoker& = delete;
    virtual ~Invoker() = default;

    [[nodiscard]] auto getId() const -> const std::string&;

    auto testSubmission(const std::string& submission_id) -> void;

    [[nodiscard]] virtual auto canTestSubmission(const std::string& submission_id) const -> bool;

  private:
    std::string id_;
    std::shared_ptr<channel_t> channel_;
};

} // namespace oink_judge::dispatcher
