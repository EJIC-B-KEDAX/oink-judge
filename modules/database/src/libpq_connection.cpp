#include "oink_judge/database/libpq_connection.h"

#include <oink_judge/logger/logger.h>

#include <boost/asio/post.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <optional>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <variant>

namespace oink_judge::database {

namespace {

using boost::asio::awaitable;
using boost::asio::use_awaitable;
using logger::logDebug;

constexpr const char* K_LOG_MODULE = "database";

auto throwIfConnectionBad(PGconn* conn) -> void {
    if (PQstatus(conn) == CONNECTION_BAD) {
        throw std::runtime_error(PQerrorMessage(conn));
    }
}

auto paramToText(const QueryParam& param) -> std::optional<std::string> {
    return std::visit(
        [](const auto& value) -> std::optional<std::string> {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, bool>) {
                return value ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::int64_t>) {
                return std::to_string(value);
            } else if constexpr (std::is_same_v<T, double>) {
                std::ostringstream stream;
                stream << value;
                return stream.str();
            } else if constexpr (std::is_same_v<T, std::string>) {
                return value;
            } else {
                return std::nullopt;
            }
        },
        param);
}

auto encodeParams(std::span<const QueryParam> params, std::vector<std::string>& storage, std::vector<const char*>& values)
    -> void {
    storage.clear();
    values.clear();
    storage.reserve(params.size());
    values.reserve(params.size());

    for (const auto& param : params) {
        const auto text = paramToText(param);
        if (text.has_value()) {
            storage.push_back(*text);
            values.push_back(storage.back().c_str());
        } else {
            values.push_back(nullptr);
        }
    }
}

auto closeDupFd(int fd) -> void {
    if (fd < 0) {
        return;
    }
    if (::close(fd) != 0) {
        logDebug(K_LOG_MODULE, "close() failed for duplicated PostgreSQL socket fd", 2);
    }
}

} // namespace

LibpqConnection::LibpqConnection() = default;

LibpqConnection::~LibpqConnection() {
    clearSocket();
    if (conn_ != nullptr) {
        PQfinish(conn_);
    }
}

auto LibpqConnection::clearSocket() -> void {
    socket_.reset();
    bound_libpq_fd_ = -1;
}

auto LibpqConnection::rebindSocketIfNeeded(const boost::asio::any_io_executor& executor) -> void {
    if (conn_ == nullptr) {
        throw std::runtime_error("connection is not initialized");
    }
    const int libpq_fd = PQsocket(conn_);
    if (libpq_fd < 0) {
        throw std::runtime_error("invalid PostgreSQL socket");
    }
    if (socket_.has_value() && bound_libpq_fd_ == libpq_fd) {
        return;
    }

    const int dup_fd = ::dup(libpq_fd);
    if (dup_fd < 0) {
        throw std::runtime_error("dup() failed for PostgreSQL socket");
    }

    if (socket_.has_value()) {
        boost::system::error_code ec;
        [[maybe_unused]] const boost::system::error_code cancel_ec = socket_->cancel(ec);
        const int old_dup = socket_->release();
        closeDupFd(old_dup);
        if (const boost::system::error_code assign_ec = socket_->assign(dup_fd, ec)) {
            closeDupFd(dup_fd);
            throw std::runtime_error("failed to assign PostgreSQL socket: " + assign_ec.message());
        }
    } else {
        socket_.emplace(executor, dup_fd);
    }
    bound_libpq_fd_ = libpq_fd;
}

auto LibpqConnection::waitForPolling(PostgresPollingStatusType status) -> awaitable<void> {
    if (!socket_.has_value()) {
        throw std::runtime_error("socket is not bound");
    }
    if (status == PGRES_POLLING_READING) {
        co_await socket_->async_wait(boost::asio::posix::stream_descriptor::wait_read, use_awaitable);
    } else if (status == PGRES_POLLING_WRITING) {
        co_await socket_->async_wait(boost::asio::posix::stream_descriptor::wait_write, use_awaitable);
    }
}

auto LibpqConnection::flushAsync() -> awaitable<void> {
    if (!socket_.has_value()) {
        throw std::runtime_error("socket is not bound");
    }
    while (PQflush(conn_) == 1) {
        co_await socket_->async_wait(boost::asio::posix::stream_descriptor::wait_write, use_awaitable);
    }
}

auto LibpqConnection::consumeResults() -> awaitable<QueryResult> {
    if (!socket_.has_value()) {
        throw std::runtime_error("socket is not bound");
    }

    QueryResult final_result;

    while (true) {
        while (PQisBusy(conn_) != 0) {
            co_await socket_->async_wait(boost::asio::posix::stream_descriptor::wait_read, use_awaitable);
            if (PQconsumeInput(conn_) == 0) {
                throw std::runtime_error(PQerrorMessage(conn_));
            }
        }

        PGresult* result = PQgetResult(conn_);
        if (result == nullptr) {
            break;
        }

        const ExecStatusType status = PQresultStatus(result);
        if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK) {
            if (PQntuples(result) > 0) {
                final_result = QueryResult(result);
                while (PQgetResult(conn_) != nullptr) {
                }
                co_return final_result;
            }
            PQclear(result);
            continue;
        }

        const std::string error = PQresultErrorMessage(result);
        PQclear(result);
        throw std::runtime_error(error);
    }

    co_return final_result;
}

auto LibpqConnection::connectAsync(std::string conninfo) -> awaitable<void> {
    logDebug(K_LOG_MODULE, "Connecting to PostgreSQL");
    logDebug(K_LOG_MODULE, "Connection info: " + conninfo);
    conninfo_ = std::move(conninfo);
    clearSocket();
    if (conn_ != nullptr) {
        PQfinish(conn_);
        conn_ = nullptr;
    }

    conn_ = PQconnectStart(conninfo_.c_str());
    if (conn_ == nullptr) {
        throw std::runtime_error("PQconnectStart failed");
    }

    PQsetnonblocking(conn_, 1);
    const auto executor = co_await boost::asio::this_coro::executor;

    while (true) {
        const PostgresPollingStatusType status = PQconnectPoll(conn_);
        if (status == PGRES_POLLING_OK) {
            break;
        }
        if (status == PGRES_POLLING_FAILED) {
            throw std::runtime_error(PQerrorMessage(conn_));
        }
        // Yield out of the async_wait completion stack before rebinding. Rebinding
        // inside that stack destroys the stream_descriptor while epoll is still
        // unwinding its completion handler.
        co_await boost::asio::post(use_awaitable);
        rebindSocketIfNeeded(executor);
        co_await waitForPolling(status);
    }

    throwIfConnectionBad(conn_);
    rebindSocketIfNeeded(executor);
    statement_cursor_ = 0;
    logDebug(K_LOG_MODULE, "PostgreSQL connection established");
}

auto LibpqConnection::reconnectAsync(std::string conninfo) -> awaitable<void> {
    logDebug(K_LOG_MODULE, "Reconnecting to PostgreSQL");
    statement_cursor_ = 0;
    co_await connectAsync(std::move(conninfo));
}

auto LibpqConnection::prepareAsync(std::string name, std::string sql) -> awaitable<void> {
    logDebug(K_LOG_MODULE, "Preparing statement on connection: " + name, 2);
    if (PQsendPrepare(conn_, name.c_str(), sql.c_str(), 0, nullptr) == 0) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }
    co_await flushAsync();
    co_await consumeResults();
}

auto LibpqConnection::unprepareAsync(std::string name) -> awaitable<void> {
    logDebug(K_LOG_MODULE, "Unpreparing statement on connection: " + name, 2);
    const std::string sql = "DEALLOCATE " + name;
    if (PQsendQuery(conn_, sql.c_str()) == 0) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }
    co_await flushAsync();
    co_await consumeResults();
}

auto LibpqConnection::syncStatements(std::span<const StatementAction> actions) -> awaitable<void> {
    if (statement_cursor_ < actions.size()) {
        logDebug(K_LOG_MODULE,
                 "Syncing " + std::to_string(actions.size() - statement_cursor_) + " statement action(s) on connection", 2);
    }
    while (statement_cursor_ < actions.size()) {
        const auto& action = actions[statement_cursor_];
        if (action.type == StatementActionType::PREPARE) {
            co_await prepareAsync(action.name, action.sql);
        } else {
            co_await unprepareAsync(action.name);
        }
        ++statement_cursor_;
    }
}

auto LibpqConnection::executePrepared(std::string stmt_name, std::span<const QueryParam> params, bool read_only)
    -> awaitable<QueryResult> {
    (void)read_only;

    logDebug(K_LOG_MODULE, "Executing prepared statement: " + stmt_name + " (params=" + std::to_string(params.size()) + ")", 2);

    std::vector<std::string> storage;
    std::vector<const char*> values;
    encodeParams(params, storage, values);

    if (PQsendQueryPrepared(conn_, stmt_name.c_str(), static_cast<int>(values.size()), values.data(), nullptr, nullptr, 0) == 0) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }

    co_await flushAsync();
    co_return co_await consumeResults();
}

auto LibpqConnection::executeSQL(std::string sql, std::span<const QueryParam> params, bool read_only) -> awaitable<QueryResult> {
    (void)read_only;

    logDebug(K_LOG_MODULE, "Executing SQL query (params=" + std::to_string(params.size()) + ")", 2);
    logDebug(K_LOG_MODULE, "SQL: " + sql, 3);

    std::vector<std::string> storage;
    std::vector<const char*> values;
    encodeParams(params, storage, values);

    if (PQsendQueryParams(conn_, sql.c_str(), static_cast<int>(values.size()), nullptr, values.data(), nullptr, nullptr, 0) ==
        0) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }

    co_await flushAsync();
    co_return co_await consumeResults();
}

auto LibpqConnection::quoteLiteral(std::string value) -> awaitable<std::string> {
    char* escaped = PQescapeLiteral(conn_, value.c_str(), value.size());
    if (escaped == nullptr) {
        throw std::runtime_error(PQerrorMessage(conn_));
    }
    std::string result = escaped;
    PQfreemem(escaped);
    co_return result;
}

auto LibpqConnection::isHealthy() const -> bool { return conn_ != nullptr && PQstatus(conn_) == CONNECTION_OK; }

} // namespace oink_judge::database
