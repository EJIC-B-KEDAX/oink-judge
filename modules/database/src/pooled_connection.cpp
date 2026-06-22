#include "oink_judge/database/pooled_connection.h"

#include "oink_judge/database/connection_pool.h"

namespace oink_judge::database {

PooledConnection::PooledConnection(ConnectionPool& pool, std::size_t slot_index) : pool_(&pool), slot_index_(slot_index) {}

PooledConnection::~PooledConnection() {
    if (pool_ != nullptr) {
        pool_->releaseSlot(slot_index_);
    }
}

PooledConnection::PooledConnection(PooledConnection&& other) noexcept : pool_(other.pool_), slot_index_(other.slot_index_) {
    other.pool_ = nullptr;
}

auto PooledConnection::operator=(PooledConnection&& other) noexcept -> PooledConnection& {
    if (this != &other) {
        if (pool_ != nullptr) {
            pool_->releaseSlot(slot_index_);
        }
        pool_ = other.pool_;
        slot_index_ = other.slot_index_;
        other.pool_ = nullptr;
    }
    return *this;
}

auto PooledConnection::connection() -> LibpqConnection& { return pool_->connectionAt(slot_index_); }

} // namespace oink_judge::database
