#pragma once
#include "oink_judge/database/libpq_connection.h"

#include <cstddef>

namespace oink_judge::database {

class ConnectionPool;

class PooledConnection {
  public:
    PooledConnection(ConnectionPool& pool, std::size_t slot_index);
    ~PooledConnection();

    PooledConnection(const PooledConnection&) = delete;
    auto operator=(const PooledConnection&) -> PooledConnection& = delete;
    PooledConnection(PooledConnection&& other) noexcept;
    auto operator=(PooledConnection&& other) noexcept -> PooledConnection&;

    [[nodiscard]] auto connection() -> LibpqConnection&;

  private:
    ConnectionPool* pool_;
    std::size_t slot_index_;
};

} // namespace oink_judge::database
