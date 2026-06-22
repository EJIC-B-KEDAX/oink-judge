#include <oink_judge/database/async_table_submissions.h>
#include <oink_judge/database/connection_pool.h>
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

auto asyncExecuteBound(std::string stmt, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::detail::executePrepared(db::ConnectionPool::instance(), std::move(stmt), std::move(params), false);
}

auto asyncExecuteReadOnlyBound(std::string stmt, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::detail::executePrepared(db::ConnectionPool::instance(), std::move(stmt), std::move(params), true);
}

auto asyncExecuteSQLBound(std::string sql, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::detail::executeSQL(db::ConnectionPool::instance(), std::move(sql), std::move(params), false);
}

auto asyncExecuteSQLReadOnlyBound(std::string sql, std::vector<db::QueryParam> params) -> awaitable<db::QueryResult> { // NOLINT
    co_return co_await db::detail::executeSQL(db::ConnectionPool::instance(), std::move(sql), std::move(params), true);
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

auto spawnAsyncExecute(std::string stmt, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(asyncExecuteBound(std::move(stmt), paramsFromPython(params)));
}

auto spawnAsyncExecuteReadOnly(std::string stmt, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(asyncExecuteReadOnlyBound(std::move(stmt), paramsFromPython(params)));
}

auto spawnAsyncExecuteSQL(std::string sql, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(asyncExecuteSQLBound(std::move(sql), paramsFromPython(params)));
}

auto spawnAsyncExecuteSQLReadOnly(std::string sql, py::list params) -> py::object { // NOLINT
    return spawnQueryResult(asyncExecuteSQLReadOnlyBound(std::move(sql), paramsFromPython(params)));
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

    m.def("async_execute", &spawnAsyncExecute, py::arg("stmt"), py::arg("params"));
    m.def("async_execute_read_only", &spawnAsyncExecuteReadOnly, py::arg("stmt"), py::arg("params"));
    m.def("async_execute_sql", &spawnAsyncExecuteSQL, py::arg("sql"), py::arg("params"));
    m.def("async_execute_sql_read_only", &spawnAsyncExecuteSQLReadOnly, py::arg("sql"), py::arg("params"));

    py::class_<db::AsyncTableSubmissions>(m, "AsyncTableSubmissions")
        .def_static("instance", &db::AsyncTableSubmissions::instance, py::return_value_policy::reference)
        .def("async_initialize", as::bindAwaitable(&db::AsyncTableSubmissions::initialize))
        .def("async_add_submission", as::bindAwaitable(&db::AsyncTableSubmissions::addSubmission), py::arg("row"))
        .def("async_load_submissions_by_user_and_problem",
             as::bindAwaitable(&db::AsyncTableSubmissions::loadSubmissionsByUserAndProblem), py::arg("username"),
             py::arg("problem_id"))
        .def("async_update_submission_verdict", as::bindAwaitable(&db::AsyncTableSubmissions::updateSubmissionVerdict),
             py::arg("submission_id"), py::arg("verdict_type"), py::arg("score"))
        .def("async_whose_submission", as::bindAwaitable(&db::AsyncTableSubmissions::whoseSubmission), py::arg("submission_id"))
        .def("async_problem_of_submission", as::bindAwaitable(&db::AsyncTableSubmissions::problemOfSubmission),
             py::arg("submission_id"))
        .def("async_language_of_submission", as::bindAwaitable(&db::AsyncTableSubmissions::languageOfSubmission),
             py::arg("submission_id"))
        .def("async_verdict_type_of_submission", as::bindAwaitable(&db::AsyncTableSubmissions::verdictTypeOfSubmission),
             py::arg("submission_id"))
        .def("async_score_of_submission", as::bindAwaitable(&db::AsyncTableSubmissions::scoreOfSubmission),
             py::arg("submission_id"))
        .def("async_set_verdict_type", as::bindAwaitable(&db::AsyncTableSubmissions::setVerdictType), py::arg("submission_id"),
             py::arg("verdict_type"))
        .def("async_set_score", as::bindAwaitable(&db::AsyncTableSubmissions::setScore), py::arg("submission_id"),
             py::arg("score"));
}
