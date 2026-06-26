#include "oink_judge/database/connection_pool.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <chrono>
#include <stdexcept>

namespace oink_judge::database {

namespace {

constexpr const char* K_LOG_MODULE = "database";

} // namespace

using boost::asio::awaitable;
using boost::asio::use_awaitable;
using config::requireHasValue;
using logger::logDebug;

ConnectionPool::ConnectionPool() : initialized_(false) {}

auto ConnectionPool::instance() -> ConnectionPool& {
    static ConnectionPool pool;
    return pool;
}

auto ConnectionPool::copyStatementActions() const -> std::vector<StatementAction> {
    std::lock_guard lock(statement_log_mutex_);
    return statement_log_.actions();
}

auto ConnectionPool::requireInitialized() -> awaitable<void> {
    if (!initialized_) {
        co_await initialize();
    }
    co_return;
}

auto ConnectionPool::initialize() -> awaitable<void> {
    if (initialized_) {
        logDebug(K_LOG_MODULE, "Connection pool already initialized");
        co_return;
    }

    config_ = requireHasValue(getDatabaseConfig());
    conninfo_ = buildConnectionString(config_);
    executor_ = co_await boost::asio::this_coro::executor;

    logDebug(K_LOG_MODULE, "Initializing connection pool (pool_min=" + std::to_string(config_.pool_min) +
                               ", pool_max=" + std::to_string(config_.pool_max) + ")");

    for (int index = 0; index < config_.pool_min; ++index) {
        co_await createSlot();
    }

    initialized_ = true;
    logDebug(K_LOG_MODULE, "Connection pool initialized with " + std::to_string(slots_.size()) + " slot(s)");
}

auto ConnectionPool::prepareStatement(std::string name, std::string sql) -> void {
    logDebug(K_LOG_MODULE, "Registering prepared statement: " + name, 2);
    std::lock_guard lock(statement_log_mutex_);
    statement_log_.appendPrepare(std::move(name), std::move(sql));
}

auto ConnectionPool::unprepareStatement(std::string name) -> void {
    logDebug(K_LOG_MODULE, "Unregistering prepared statement: " + name, 2);
    std::lock_guard lock(statement_log_mutex_);
    statement_log_.appendUnprepare(std::move(name));
}

auto ConnectionPool::createSlot() -> awaitable<std::size_t> {
    auto connection = std::make_unique<LibpqConnection>();
    co_await connection->connectAsync(conninfo_);

    const auto actions = copyStatementActions();
    co_await connection->syncStatements(actions);

    std::lock_guard lock(pool_mutex_);
    const std::size_t index = slots_.size();
    slots_.push_back(Slot{.connection = std::move(connection), .in_use = false});
    logDebug(K_LOG_MODULE, "Created pool slot " + std::to_string(index));
    co_return index;
}

auto ConnectionPool::tryAcquireExistingSlot() -> std::optional<std::size_t> {
    for (std::size_t index = 0; index < slots_.size(); ++index) {
        if (!slots_[index].in_use) {
            slots_[index].in_use = true;
            return index;
        }
    }
    return std::nullopt;
}

auto ConnectionPool::waitForAvailableSlot() -> awaitable<std::size_t> {
    for (;;) {
        {
            std::lock_guard lock(pool_mutex_);
            if (auto slot = tryAcquireExistingSlot()) {
                co_return *slot;
            }
        }

        boost::asio::steady_timer timer(executor_);
        timer.expires_after(std::chrono::milliseconds(1));
        co_await timer.async_wait(use_awaitable);
    }
}

auto ConnectionPool::acquire() -> awaitable<PooledConnection> {
    co_await requireInitialized();

    logDebug(K_LOG_MODULE, "Acquiring connection from pool", 2);

    std::size_t slot_index = 0;
    bool has_slot = false;
    bool should_create = false;

    {
        std::lock_guard lock(pool_mutex_);
        if (auto slot = tryAcquireExistingSlot()) {
            slot_index = *slot;
            has_slot = true;
        } else if (static_cast<int>(slots_.size()) < config_.pool_max) {
            should_create = true;
        }
    }

    if (should_create) {
        slot_index = co_await createSlot();
        std::lock_guard lock(pool_mutex_);
        slots_[slot_index].in_use = true;
        has_slot = true;
    }

    if (!has_slot) {
        slot_index = co_await waitForAvailableSlot();
        logDebug(K_LOG_MODULE, "Acquired pool slot " + std::to_string(slot_index) + " after waiting", 2);
    } else if (should_create) {
        logDebug(K_LOG_MODULE, "Acquired newly created pool slot " + std::to_string(slot_index), 2);
    } else {
        logDebug(K_LOG_MODULE, "Acquired existing pool slot " + std::to_string(slot_index), 2);
    }

    const auto actions = copyStatementActions();
    co_await connectionAt(slot_index).syncStatements(actions);
    co_return PooledConnection(*this, slot_index);
}

auto ConnectionPool::connectionAt(std::size_t slot_index) -> LibpqConnection& {
    std::lock_guard lock(pool_mutex_);
    if (slot_index >= slots_.size()) {
        throw std::runtime_error("invalid pool slot index");
    }
    return *slots_[slot_index].connection;
}

auto ConnectionPool::releaseSlot(std::size_t slot_index) -> void {
    std::lock_guard lock(pool_mutex_);
    if (slot_index >= slots_.size()) {
        throw std::runtime_error("invalid pool slot index");
    }
    slots_[slot_index].in_use = false;
    logDebug(K_LOG_MODULE, "Released pool slot " + std::to_string(slot_index), 2);
}

} // namespace oink_judge::database
