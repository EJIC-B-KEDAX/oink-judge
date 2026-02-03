// Simple test demonstrating Boost.Asio co_spawn with an awaitable
// Prints from the awaitable and from main after co_spawn is called.

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;

awaitable<void> awaitable_function()
{
	std::cout << "Awaitable function output" << std::endl;
	co_return;
}

int main()
{
	boost::asio::io_context ioc{1};

	// Spawn the coroutine onto the io_context's executor.
	co_spawn(ioc, awaitable_function(), detached);

	// Print immediately after co_spawn, as requested.
	std::cout << "Main output" << std::endl;

	// Run the context so the awaitable actually executes.
	ioc.run();

	return 0;
}
