#pragma once
#include "oink_judge/database/config_utils.h"
#include "oink_judge/database/pooled_connection.h"
#include "oink_judge/database/statement_action_log.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

class ConnectionPool {
  public:
    static auto instance() -> ConnectionPool&;

    auto initialize() -> awaitable<void>;

    auto prepareStatement(std::string name, std::string sql) -> void;
    auto unprepareStatement(std::string name) -> void;

    auto acquire() -> awaitable<PooledConnection>;

  private:
    friend class PooledConnection;

    struct Slot {
        std::unique_ptr<LibpqConnection> connection;
        bool in_use = false;
    };

    ConnectionPool();

    auto requireInitialized() -> awaitable<void>;
    auto connectionAt(std::size_t slot_index) -> LibpqConnection&;
    auto releaseSlot(std::size_t slot_index) -> void;
    auto createSlot() -> awaitable<std::size_t>;
    auto tryAcquireExistingSlot() -> std::optional<std::size_t>;
    auto waitForAvailableSlot() -> awaitable<std::size_t>;
    auto copyStatementActions() const -> std::vector<StatementAction>;

    DatabaseConfig config_;
    std::string conninfo_;
    bool initialized_;
    StatementActionLog statement_log_;
    std::vector<Slot> slots_;
    mutable std::mutex pool_mutex_;
    mutable std::mutex statement_log_mutex_;
    boost::asio::any_io_executor executor_;
};

} // namespace oink_judge::database
