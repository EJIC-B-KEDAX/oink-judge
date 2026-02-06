#pragma once
#include <boost/asio.hpp>
#include <oink_judge/factory/parameterized_type_factory.hpp>

namespace oink_judge::socket {

using boost::asio::awaitable;
using boost::asio::ip::tcp;

template <typename... Args>
using awaitable_packed_types_t = boost::asio::async_result<boost::asio::use_awaitable_t<>,
                                                           void(boost::system::error_code, std::decay_t<Args>...)>::return_type;

class Session {
  public:
    using callback_t = std::function<void(std::error_code, std::any)>;

    Session(const Session&) = delete;
    auto operator=(const Session&) -> Session& = delete;
    Session(Session&&) = delete;
    auto operator=(Session&&) -> Session& = delete;
    virtual ~Session() = default;

    virtual auto start(std::string start_message) -> awaitable<void> = 0;
    virtual auto sendMessage(std::string message) -> awaitable<void> = 0;
    virtual auto receiveMessage() -> awaitable<void> = 0;
    virtual auto close() -> void = 0;

    template <typename... Args> auto request(std::string message) -> awaitable_packed_types_t<Args...>;
    template <typename F, typename... Args> auto request(const std::string& message, F&& callback) -> void;

    virtual auto requestInternal(const std::string& message, const callback_t& callback) -> void = 0;

    virtual auto getExecutor() -> boost::asio::any_io_executor = 0;

  protected:
    Session() = default;
};

using SessionFactory = factory::ParameterizedTypeFactory<std::shared_ptr<Session>, tcp::socket>;

} // namespace oink_judge::socket

#include "oink_judge/socket/session.inl"
