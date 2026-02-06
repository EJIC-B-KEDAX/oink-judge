#pragma once
#include <boost/asio.hpp>

namespace oink_judge::socket {

class BoostIOContext {
  public:
    BoostIOContext(const BoostIOContext&) = delete;
    auto operator=(const BoostIOContext&) -> BoostIOContext& = delete;
    BoostIOContext(BoostIOContext&&) = delete;
    auto operator=(BoostIOContext&&) -> BoostIOContext& = delete;
    ~BoostIOContext();

    static auto instance() -> boost::asio::io_context&;

  private:
    BoostIOContext() = default;
};

} // namespace oink_judge::socket
