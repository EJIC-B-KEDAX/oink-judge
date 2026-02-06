#include "oink_judge/socket/boost_io_context.h"

namespace oink_judge::socket {

auto BoostIOContext::instance() -> boost::asio::io_context& {
    static boost::asio::io_context instance;
    return instance;
}

} // namespace oink_judge::socket
