#include "oink_judge/utils/async.h"

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/process/extend.hpp>
#include <oink_judge/logger/logger.h>

namespace oink_judge::utils::async {

auto awaitableSystem(std::string command) -> awaitable<int> {
    using namespace boost::asio;
    namespace bp = boost::process;

    auto executor = co_await this_coro::executor;

    // Use bp::on_exit and boost::asio::async_initiate to make the notification
    // awaitable. We keep the child alive by storing it inside a shared holder
    // that the on_exit callback owns until it runs.

    int exit_code =
        co_await boost::asio::async_initiate<decltype(boost::asio::use_awaitable), void(boost::system::error_code, int)>(
            [command = std::move(command), executor](auto&& handler) mutable -> auto {
                using HandlerType = std::decay_t<decltype(handler)>;

                // Create a type-specific holder that stores the completion
                // handler (which may be move-only) and the child. Storing the
                // handler inside a heap-allocated shared_ptr avoids requiring
                // it to be copy-constructible.
                struct LocalHolder {
                    std::shared_ptr<HandlerType> completion;
                    std::shared_ptr<bp::child> child;
                };

                auto holder = std::make_shared<LocalHolder>();
                holder->completion = std::make_shared<HandlerType>(std::forward<HandlerType>(handler));

                // Build the on_exit callback that will post the stored
                // completion handler into the coroutine's executor (so it
                // runs on the expected asio context), converting
                // std::error_code to boost::system::error_code, and then
                // release the child.
                auto on_exit_cb = [holder, executor](int exit_code, std::error_code const& ec) mutable -> void {
                    try {
                        boost::system::error_code bec(ec);
                        boost::asio::post(executor, [holder, bec, exit_code]() mutable -> void {
                            try {
                                (*holder->completion)(bec, exit_code);
                            } catch (std::exception& e) {
                                logger::logMessage("async_utils", 1,
                                                   "Exception in async completion handler: " + std::string(e.what()),
                                                   logger::LogType::ERROR);
                            }
                        });
                    } catch (std::exception& e) {
                        logger::logMessage("async_utils", 1,
                                           "Exception posting async completion handler: " + std::string(e.what()),
                                           logger::LogType::ERROR);
                    }
                    holder->child.reset();
                };

                // Let Boost.Process use its default reactor and ensure
                // the completion runs on the coroutine's executor by posting
                // into it from the on_exit callback.
                holder->child = std::make_shared<bp::child>("/bin/sh", "-c", command, bp::std_out > bp::null,
                                                            bp::std_err > bp::null, bp::on_exit(on_exit_cb));
            },
            boost::asio::use_awaitable);

    co_return exit_code;
}

} // namespace oink_judge::utils::async
