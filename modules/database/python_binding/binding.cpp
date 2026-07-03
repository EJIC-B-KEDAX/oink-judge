#include <oink_judge/database/table_submissions.h>
#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/pooled_connection.h>
#include <oink_judge/database/query.h>
#include <oink_judge/database/query_param.h>
#include <oink_judge/database/query_result.h>
#include <oink_judge/python_binding/awaitable_support/awaitable_binder.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>

namespace py = pybind11;
namespace db = oink_judge::database;
namespace as = oink_judge::python_bindings::awaitable_support;

using boost::asio::awaitable;

namespace {

auto paramsFromPython(const py::list& py_params) -> std::vector<db::QueryParam> {
    std::vector<db::QueryParam> params;
    params.reserve(py_params.size());
    for (const auto& item : py_params) {
        if (item.is_none()) {
            params.emplace_back(std::monostate{});
        } else if (py::isinstance<py::bool_>(item)) {
            params.emplace_back(item.cast<bool>());
        } else if (py::isinstance<py::int_>(item)) {
            params.emplace_back(item.cast<std::int64_t>());
        } else if (py::isinstance<py::float_>(item)) {
            params.emplace_back(item.cast<double>());
        } else if (py::isinstance<py::str>(item)) {
            params.emplace_back(item.cast<std::string>());
        } else {
            throw std::runtime_error("unsupported query parameter type");
        }
    }
    return params;
}

auto awaitQueryResultAndResolve(PyObject* raw_future, awaitable<db::QueryResult> task) -> awaitable<void> {
    try {
        auto result = co_await std::move(task);
        py::gil_scoped_acquire gil;
        py::reinterpret_steal<py::object>(raw_future)
            .attr("set_result")(py::cast(std::make_unique<db::QueryResult>(std::move(result))));
    } catch (...) {
        as::detail::setFutureException(raw_future, std::current_exception());
    }
    co_return;
}

auto spawnQueryResult(awaitable<db::QueryResult> task) -> py::object {
    auto& bridge = as::AwaitableBridge::current();
    auto future = py::reinterpret_steal<py::object>(bridge.createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(bridge.ioContext(), awaitQueryResultAndResolve(raw, std::move(task)), boost::asio::detached);
    return future;
}

template <typename T>
auto awaitValueAndResolve(PyObject* raw_future, awaitable<T> task) -> awaitable<void> {
    try {
        auto result = co_await std::move(task);
        py::gil_scoped_acquire gil;
        py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::cast(std::move(result)));
    } catch (...) {
        as::detail::setFutureException(raw_future, std::current_exception());
    }
    co_return;
}

template <typename T>
auto spawnAwaitable(awaitable<T> task) -> py::object {
    auto& bridge = as::AwaitableBridge::current();
    auto future = py::reinterpret_steal<py::object>(bridge.createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(bridge.ioContext(), awaitValueAndResolve(raw, std::move(task)), boost::asio::detached);
    return future;
}

auto pooledExecuteBound(std::string stmt, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::execute(db::ConnectionPool::instance(), std::move(stmt), params);
}

auto pooledExecuteReadOnlyBound(std::string stmt, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::executeReadOnly(db::ConnectionPool::instance(), std::move(stmt), params);
}

auto pooledExecuteSQLBound(std::string sql, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::executeSQL(db::ConnectionPool::instance(), std::move(sql), std::span<const db::QueryParam>(params));
}

auto pooledExecuteSQLReadOnlyBound(std::string sql, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::executeSQLReadOnly(db::ConnectionPool::instance(), std::move(sql),
                                              std::span<const db::QueryParam>(params));
}

auto connectionExecuteBound(db::PooledConnection& connection, std::string stmt, std::vector<db::QueryParam> params) // NOLINT
    -> awaitable<db::QueryResult> {
    co_return co_await db::execute(connection.connection(), std::move(stmt), params);
}

auto connectionExecuteReadOnlyBound(db::PooledConnection& connection, std::string stmt, std::vector<db::QueryParam> params) // NOLINT
    -> awaitable<db::QueryResult> {
    co_return co_await db::executeReadOnly(connection.connection(), std::move(stmt), params);
}

auto connectionExecuteSQLBound(db::PooledConnection& connection, std::string sql, std::vector<db::QueryParam> params) // NOLINT
    -> awaitable<db::QueryResult> {
    co_return co_await db::executeSQL(connection.connection(), std::move(sql), std::span<const db::QueryParam>(params));
}

auto connectionExecuteSQLReadOnlyBound(db::PooledConnection& connection, std::string sql, // NOLINT
                                       std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> {
    co_return co_await db::executeSQLReadOnly(connection.connection(), std::move(sql),
                                              std::span<const db::QueryParam>(params));
}

auto spawnPooledExecute(std::string stmt, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(pooledExecuteBound(std::move(stmt), paramsFromPython(params)));
}

auto spawnPooledExecuteReadOnly(std::string stmt, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(pooledExecuteReadOnlyBound(std::move(stmt), paramsFromPython(params)));
}

auto spawnPooledExecuteSQL(std::string sql, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(pooledExecuteSQLBound(std::move(sql), paramsFromPython(params)));
}

auto spawnPooledExecuteSQLReadOnly(std::string sql, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(pooledExecuteSQLReadOnlyBound(std::move(sql), paramsFromPython(params)));
}

auto spawnConnectionExecute(db::PooledConnection& connection, std::string stmt, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(connectionExecuteBound(connection, std::move(stmt), paramsFromPython(params)));
}

auto spawnConnectionExecuteReadOnly(db::PooledConnection& connection, std::string stmt, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(connectionExecuteReadOnlyBound(connection, std::move(stmt), paramsFromPython(params)));
}

auto spawnConnectionExecuteSQL(db::PooledConnection& connection, std::string sql, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(connectionExecuteSQLBound(connection, std::move(sql), paramsFromPython(params)));
}

auto spawnConnectionExecuteSQLReadOnly(db::PooledConnection& connection, std::string sql, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(connectionExecuteSQLReadOnlyBound(connection, std::move(sql), paramsFromPython(params)));
}

auto spawnAcquireConnection() -> py::object {
    return spawnAwaitable(db::ConnectionPool::instance().acquire());
}

} // namespace

PYBIND11_MODULE(pybind11_database, m) {
    py::class_<db::SubmissionRow>(m, "SubmissionRow")
        .def(py::init<>())
        .def_readwrite("id", &db::SubmissionRow::id)
        .def_readwrite("username", &db::SubmissionRow::username)
        .def_readwrite("problem_id", &db::SubmissionRow::problem_id)
        .def_readwrite("language", &db::SubmissionRow::language)
        .def_readwrite("verdict_type", &db::SubmissionRow::verdict_type)
        .def_readwrite("score", &db::SubmissionRow::score)
        .def_readwrite("send_time", &db::SubmissionRow::send_time);

    py::class_<db::QueryField>(m, "QueryField")
        .def("is_null", &db::QueryField::isNull)
        .def("as_string", &db::QueryField::as<std::string>)
        .def("as_int", &db::QueryField::as<int>)
        .def("as_int64", &db::QueryField::as<std::int64_t>)
        .def("as_double", &db::QueryField::as<double>)
        .def("as_bool", &db::QueryField::as<bool>);

    py::class_<db::QueryRow>(m, "QueryRow")
        .def("size", &db::QueryRow::size)
        .def("field", &db::QueryRow::field, py::arg("column"), py::keep_alive<1, 0>())
        .def(
            "__getitem__",
            [](const db::QueryRow& self, const py::object& key) -> db::QueryField {
                if (py::isinstance<py::int_>(key)) {
                    return self[key.cast<int>()];
                }
                if (py::isinstance<py::str>(key)) {
                    return self[key.cast<std::string>()];
                }
                throw std::runtime_error("column key must be int or str");
            },
            py::arg("column"), py::keep_alive<1, 0>());

    py::class_<db::QueryResult, std::unique_ptr<db::QueryResult>>(m, "QueryResult")
        .def("empty", &db::QueryResult::empty)
        .def("size", &db::QueryResult::size)
        .def("__len__", &db::QueryResult::size)
        .def("row", &db::QueryResult::row, py::arg("index"), py::keep_alive<1, 0>())
        .def(
            "__getitem__", [](const db::QueryResult& self, int index) -> db::QueryRow { return self[index]; }, py::arg("index"),
            py::keep_alive<1, 0>());

    py::class_<db::ConnectionPool>(m, "ConnectionPool")
        .def_static("instance", &db::ConnectionPool::instance, py::return_value_policy::reference)
        .def("async_initialize", as::bindAwaitable(&db::ConnectionPool::initialize))
        .def("prepare_statement", &db::ConnectionPool::prepareStatement, py::arg("name"), py::arg("sql"))
        .def("unprepare_statement", &db::ConnectionPool::unprepareStatement, py::arg("name"));

    py::class_<db::PooledConnection>(m, "DbConnection")
        .def("__repr__", [](const db::PooledConnection&) { return "<DbConnection>"; })
        .def("execute", &spawnConnectionExecute, py::arg("stmt"), py::arg("params"))
        .def("execute_read_only", &spawnConnectionExecuteReadOnly, py::arg("stmt"), py::arg("params"))
        .def("execute_sql", &spawnConnectionExecuteSQL, py::arg("sql"), py::arg("params"))
        .def("execute_sql_read_only", &spawnConnectionExecuteSQLReadOnly, py::arg("sql"), py::arg("params"));

    m.def("acquire_connection", &spawnAcquireConnection);
    m.def("async_execute", &spawnPooledExecute, py::arg("stmt"), py::arg("params"));
    m.def("async_execute_read_only", &spawnPooledExecuteReadOnly, py::arg("stmt"), py::arg("params"));
    m.def("async_execute_sql", &spawnPooledExecuteSQL, py::arg("sql"), py::arg("params"));
    m.def("async_execute_sql_read_only", &spawnPooledExecuteSQLReadOnly, py::arg("sql"), py::arg("params"));

    py::class_<db::TableSubmissions>(m, "TableSubmissions")
        .def_static("instance", &db::TableSubmissions::instance, py::return_value_policy::reference)
        .def("initialize", as::bindAwaitable(&db::TableSubmissions::initialize))
        .def("add_submission", as::bindAwaitable(&db::TableSubmissions::addSubmission), py::arg("row"))
        .def("load_submissions_by_user_and_problem",
             as::bindAwaitable(&db::TableSubmissions::loadSubmissionsByUserAndProblem), py::arg("username"),
             py::arg("problem_id"))
        .def("update_submission_verdict", as::bindAwaitable(&db::TableSubmissions::updateSubmissionVerdict),
             py::arg("submission_id"), py::arg("verdict_type"), py::arg("score"))
        .def("whose_submission", as::bindAwaitable(&db::TableSubmissions::whoseSubmission), py::arg("submission_id"))
        .def("problem_of_submission", as::bindAwaitable(&db::TableSubmissions::problemOfSubmission),
             py::arg("submission_id"))
        .def("language_of_submission", as::bindAwaitable(&db::TableSubmissions::languageOfSubmission),
             py::arg("submission_id"))
        .def("verdict_type_of_submission", as::bindAwaitable(&db::TableSubmissions::verdictTypeOfSubmission),
             py::arg("submission_id"))
        .def("score_of_submission", as::bindAwaitable(&db::TableSubmissions::scoreOfSubmission), py::arg("submission_id"))
        .def("set_verdict_type", as::bindAwaitable(&db::TableSubmissions::setVerdictType), py::arg("submission_id"),
             py::arg("verdict_type"))
        .def("set_score", as::bindAwaitable(&db::TableSubmissions::setScore), py::arg("submission_id"), py::arg("score"));
}
