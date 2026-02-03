#include <iostream>
#include <boost/asio.hpp>
#include "utils/async.h"

using namespace boost::asio;
using oink_judge::utils::async::awaitable_system;

int main() {
    io_context io;

    co_spawn(io.get_executor(), []() -> awaitable<void> {
        int rc = co_await awaitable_system("ls > logs/awaitable_system_test_output.txt");
        std::cout << "child rc=" << rc << std::endl;
        co_return;
    }, detached);

    io.run();
    return 0;
}
