#pragma once
#include <boost/asio.hpp>

namespace oink_judge::utils::async {

using boost::asio::awaitable;

auto awaitableSystem(std::string) -> awaitable<int>;

} // namespace oink_judge::utils::async
