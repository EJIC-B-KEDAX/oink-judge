#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/database_executor_interface.h>
#include <oink_judge/database/execute_options.h>
#include <oink_judge/database/pooled_connection.h>
#include <oink_judge/database/query.h>
#include <oink_judge/database/query_param.h>
#include <oink_judge/database/query_result.h>
#include <oink_judge/database/statements.h>
#include <oink_judge/database/table_execute_options.h>
#include <oink_judge/database/table_submissions.h>
#include <oink_judge/python_binding/awaitable_support/awaitable_binder.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace py = pybind11;
namespace db = oink_judge::database;
namespace as = oink_judge::python_bindings::awaitable_support;

using boost::asio::awaitable;

namespace {

using PrepareStatementsFn = awaitable<void> (db::DatabaseExecutorInterface::*)(db::StatementsBlock);
using UnprepareStatementsFn = awaitable<void> (db::DatabaseExecutorInterface::*)(std::string);
using GetConnectionFn = awaitable<std::shared_ptr<db::DatabaseExecutorInterface>> (db::DatabaseExecutorInterface::*)();
using QuoteFn = awaitable<std::string> (db::DatabaseExecutorInterface::*)(std::string);

constexpr const char* K_READ_ONLY = "read_only";
constexpr const char* K_TIMEOUT_SEC = "timeout_sec";
constexpr const char* K_RETRIES = "retries";
constexpr const char* K_EXECUTOR = "executor";

auto queryParamFromPython(const py::handle& item) -> db::QueryParam {
    if (item.is_none()) {
        return std::monostate{};
    }
    if (py::isinstance<py::bool_>(item)) {
        return item.cast<bool>();
    }
    if (py::isinstance<py::int_>(item)) {
        return item.cast<std::int64_t>();
    }
    if (py::isinstance<py::float_>(item)) {
        return item.cast<double>();
    }
    if (py::isinstance<py::str>(item)) {
        return item.cast<std::string>();
    }
    throw std::runtime_error("unsupported query parameter type");
}

auto paramsFromArgs(const py::args& args) -> std::vector<db::QueryParam> {
    std::vector<db::QueryParam> params;
    params.reserve(args.size());
    for (const auto& item : args) {
        params.push_back(queryParamFromPython(item));
    }
    return params;
}

auto popOptionalBool(py::dict& kwargs, const char* key) -> std::optional<bool> {
    if (!kwargs.contains(key)) {
        return std::nullopt;
    }
    py::object value = kwargs.attr("pop")(key);
    if (value.is_none()) {
        return std::nullopt;
    }
    return value.cast<bool>();
}

auto popOptionalInt(py::dict& kwargs, const char* key) -> std::optional<int> {
    if (!kwargs.contains(key)) {
        return std::nullopt;
    }
    py::object value = kwargs.attr("pop")(key);
    if (value.is_none()) {
        return std::nullopt;
    }
    return value.cast<int>();
}

auto popObject(py::dict& kwargs, const char* key, py::object default_value = py::none()) -> py::object {
    if (!kwargs.contains(key)) {
        return default_value;
    }
    return kwargs.attr("pop")(key);
}

auto ensureNoExtraKwargs(const py::dict& kwargs) -> void {
    if (kwargs.empty()) {
        return;
    }
    const auto keys = py::list(kwargs.attr("keys")());
    throw std::runtime_error("unexpected keyword argument: " + py::str(keys[0]).cast<std::string>());
}

auto makeExecuteOptions(std::optional<bool> read_only, std::optional<int> timeout_sec, std::optional<int> retries)
    -> db::ExecuteOptions {
    return db::ExecuteOptions{.read_only = read_only, .timeout_sec = timeout_sec, .retries = retries};
}

auto executeOptionsFromKwargs(py::dict& kwargs) -> db::ExecuteOptions {
    return makeExecuteOptions(popOptionalBool(kwargs, K_READ_ONLY), popOptionalInt(kwargs, K_TIMEOUT_SEC),
                              popOptionalInt(kwargs, K_RETRIES));
}

auto executorFromPython(const py::object& db) -> std::shared_ptr<db::DatabaseExecutorInterface> {
    if (db.is_none()) {
        return db::getDefaultExecutor();
    }
    if (py::isinstance<db::DatabaseExecutorInterface>(db)) {
        return db.cast<std::shared_ptr<db::DatabaseExecutorInterface>>();
    }
    throw std::runtime_error("db must be None or a DatabaseExecutor");
}

auto tableExecuteOptionsFromKwargs(py::kwargs kwargs) -> db::TableExecuteOptions {
    py::dict kwargs_dict = std::move(kwargs);
    auto options = db::TableExecuteOptions{};
    options.execute_options = executeOptionsFromKwargs(kwargs_dict);
    const auto executor = popObject(kwargs_dict, K_EXECUTOR);
    if (!executor.is_none()) {
        options.executor = executorFromPython(executor);
    }
    ensureNoExtraKwargs(kwargs_dict);
    return options;
}

auto awaitQueryResultAndResolve(PyObject* raw_future, py::object owner_py, awaitable<db::QueryResult> task) -> awaitable<void> {
    (void)owner_py;
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

auto spawnQueryResult(py::object owner_py, awaitable<db::QueryResult> task) -> py::object {
    auto& bridge = as::AwaitableBridge::current();
    auto future = py::reinterpret_steal<py::object>(bridge.createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(bridge.ioContext(), awaitQueryResultAndResolve(raw, std::move(owner_py), std::move(task)), boost::asio::detached);
    return future;
}

auto spawnExecute(std::string stmt, const py::args& params, py::kwargs kwargs) -> py::object {
    py::dict kwargs_dict = std::move(kwargs);
    auto options = executeOptionsFromKwargs(kwargs_dict);
    auto executor = executorFromPython(popObject(kwargs_dict, K_EXECUTOR));
    ensureNoExtraKwargs(kwargs_dict);
    return spawnQueryResult(py::cast(executor), executor->execute(options, std::move(stmt), paramsFromArgs(params)));
}

auto spawnExecuteSQL(std::string sql, const py::args& params, py::kwargs kwargs) -> py::object {
    py::dict kwargs_dict = std::move(kwargs);
    auto options = executeOptionsFromKwargs(kwargs_dict);
    auto executor = executorFromPython(popObject(kwargs_dict, K_EXECUTOR));
    ensureNoExtraKwargs(kwargs_dict);
    return spawnQueryResult(py::cast(executor), executor->executeSQL(options, std::move(sql), paramsFromArgs(params)));
}

auto spawnQuote(std::string value, py::kwargs kwargs) -> py::object {
    py::dict kwargs_dict = std::move(kwargs);
    auto executor = executorFromPython(popObject(kwargs_dict, K_EXECUTOR));
    ensureNoExtraKwargs(kwargs_dict);
    return as::spawnMethod(py::cast(executor), executor->quote(std::move(value)));
}

auto spawnAcquireConnection() -> py::object {
    auto executor = db::getDefaultExecutor();
    return as::spawnMethod(py::cast(executor), executor->getConnection());
}

auto bindExecutorMethods(py::class_<db::DatabaseExecutorInterface, std::shared_ptr<db::DatabaseExecutorInterface>>& cls) -> void {
    cls.def(
           "execute",
           [](db::DatabaseExecutorInterface& self, std::string stmt, const py::args& params, py::kwargs kwargs) -> py::object {
               py::dict kwargs_dict = std::move(kwargs);
               auto options = executeOptionsFromKwargs(kwargs_dict);
               ensureNoExtraKwargs(kwargs_dict);
               auto owner = py::cast(self.shared_from_this());
               return spawnQueryResult(owner, self.execute(options, std::move(stmt), paramsFromArgs(params)));
           },
           py::arg("stmt"))
        .def(
            "execute_sql",
            [](db::DatabaseExecutorInterface& self, std::string sql, const py::args& params, py::kwargs kwargs) -> py::object {
                py::dict kwargs_dict = std::move(kwargs);
                auto options = executeOptionsFromKwargs(kwargs_dict);
                ensureNoExtraKwargs(kwargs_dict);
                auto owner = py::cast(self.shared_from_this());
                return spawnQueryResult(owner, self.executeSQL(options, std::move(sql), paramsFromArgs(params)));
            },
            py::arg("sql"))
        .def("get_connection", as::bindAwaitable(static_cast<GetConnectionFn>(&db::DatabaseExecutorInterface::getConnection)))
        .def("quote", as::bindAwaitable(static_cast<QuoteFn>(&db::DatabaseExecutorInterface::quote)), py::arg("value"))
        .def("prepare_statements",
             as::bindAwaitable(static_cast<PrepareStatementsFn>(&db::DatabaseExecutorInterface::prepareStatements)),
             py::arg("statements_block"))
        .def("unprepare_statements",
             as::bindAwaitable(static_cast<UnprepareStatementsFn>(&db::DatabaseExecutorInterface::unprepareStatements)),
             py::arg("block_name"))
        .def("is_prepared", &db::DatabaseExecutorInterface::isPrepared, py::arg("block_name"))
        .def(
            "prepare_statement",
            [](db::DatabaseExecutorInterface& self, db::Statement statement) -> py::object {
                return as::spawnMethod(py::cast(self.shared_from_this()), self.prepareStatement(std::move(statement)));
            },
            py::arg("statement"))
        .def(
            "prepare_statement",
            [](db::DatabaseExecutorInterface& self, std::string name, std::string sql) -> py::object {
                return as::spawnMethod(py::cast(self.shared_from_this()),
                                       self.prepareStatement(db::Statement(std::move(name), std::move(sql))));
            },
            py::arg("name"), py::arg("sql"))
        .def(
            "unprepare_statement",
            [](db::DatabaseExecutorInterface& self, const std::string& statement_name) -> py::object {
                return as::spawnMethod(py::cast(self.shared_from_this()), self.unprepareStatement(statement_name));
            },
            py::arg("statement_name"));
}

} // namespace

PYBIND11_MODULE(pybind11_database, m) {
    py::class_<db::ExecuteOptions>(m, "ExecuteOptions")
        .def(py::init<>())
        .def(py::init([](std::optional<bool> read_only, std::optional<int> timeout_sec, std::optional<int> retries)
                          -> db::ExecuteOptions { return makeExecuteOptions(read_only, timeout_sec, retries); }),
             py::arg("read_only") = std::nullopt, py::arg("timeout_sec") = std::nullopt, py::arg("retries") = std::nullopt)
        .def_readwrite("read_only", &db::ExecuteOptions::read_only)
        .def_readwrite("timeout_sec", &db::ExecuteOptions::timeout_sec)
        .def_readwrite("retries", &db::ExecuteOptions::retries)
        .def("fill_with_defaults", py::overload_cast<>(&db::ExecuteOptions::fillWithDefaults))
        .def("fill_with_defaults", py::overload_cast<const db::ExecuteOptions&>(&db::ExecuteOptions::fillWithDefaults),
             py::arg("default_options"));

    m.def("get_default_execute_options", &db::getDefaultExecuteOptions);

    py::class_<db::Statement>(m, "Statement")
        .def(py::init<std::string, std::string>(), py::arg("name"), py::arg("sql"))
        .def_property_readonly("name", &db::Statement::name)
        .def_property_readonly("sql", &db::Statement::sql);

    py::class_<db::StatementsBlock>(m, "StatementsBlock")
        .def(py::init<std::string>(), py::arg("block_name"))
        .def_property_readonly("name", &db::StatementsBlock::name)
        .def("add_statement", &db::StatementsBlock::addStatement, py::arg("statement"))
        .def("get_statement", &db::StatementsBlock::getStatement, py::arg("name"), py::return_value_policy::reference_internal)
        .def("statements", &db::StatementsBlock::statements, py::return_value_policy::reference_internal);

    py::class_<db::DatabaseExecutorInterface, std::shared_ptr<db::DatabaseExecutorInterface>> database_executor(
        m, "DatabaseExecutor");
    bindExecutorMethods(database_executor);

    py::class_<db::ConnectionPool, db::DatabaseExecutorInterface, std::shared_ptr<db::ConnectionPool>>(m, "ConnectionPool")
        .def(py::init<>())
        .def_readonly_static("REGISTERED_NAME", &db::ConnectionPool::REGISTERED_NAME)
        .def("initialize", as::bindAwaitable(&db::ConnectionPool::initialize))
        .def("acquire", as::bindAwaitable(&db::ConnectionPool::acquire));

    py::class_<db::PooledConnection, db::DatabaseExecutorInterface, std::shared_ptr<db::PooledConnection>>(m, "DbConnection")
        .def("__repr__", [](const db::PooledConnection&) -> const char* { return "<DbConnection>"; });

    py::class_<db::TableExecuteOptions>(m, "TableExecuteOptions")
        .def(py::init<>())
        .def(py::init([](std::optional<bool> read_only, std::optional<int> timeout_sec, std::optional<int> retries,
                         std::shared_ptr<db::DatabaseExecutorInterface> executor) -> db::TableExecuteOptions {
                 return db::TableExecuteOptions{.execute_options = makeExecuteOptions(read_only, timeout_sec, retries),
                                                .executor = std::move(executor)};
             }),
             py::arg("read_only") = std::nullopt, py::arg("timeout_sec") = std::nullopt, py::arg("retries") = std::nullopt,
             py::arg("executor") = std::shared_ptr<db::DatabaseExecutorInterface>{})
        .def_readwrite("execute_options", &db::TableExecuteOptions::execute_options)
        .def_readwrite("executor", &db::TableExecuteOptions::executor)
        .def("fill_with_defaults", py::overload_cast<>(&db::TableExecuteOptions::fillWithDefaults))
        .def("fill_with_defaults", py::overload_cast<const db::TableExecuteOptions&>(&db::TableExecuteOptions::fillWithDefaults),
             py::arg("default_options"));

    m.def("get_default_executor_option", &db::getDefaultExecutorOption);
    m.def("create_database_executor", &db::createDatabaseExecutor, py::arg("name"));
    m.def("get_default_executor", &db::getDefaultExecutor);
    m.def("register_database_executor_types", &db::registerDatabaseExecutorTypes);

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

    m.def("acquire_connection", &spawnAcquireConnection);
    m.def("execute", &spawnExecute, py::arg("stmt"));
    m.def("execute_sql", &spawnExecuteSQL, py::arg("sql"));
    m.def("quote", &spawnQuote, py::arg("value"));

    py::class_<db::TableSubmissions>(m, "TableSubmissions")
        .def_static("instance", &db::TableSubmissions::instance, py::return_value_policy::reference)
        .def_readonly_static("STATEMENTS_BLOCK_NAME", &db::TableSubmissions::STATEMENTS_BLOCK_NAME)
        .def("initialize",
             [](db::TableSubmissions& self, py::kwargs kwargs) -> py::object {
                 return as::spawnMethod(py::cast(self), self.initialize(tableExecuteOptionsFromKwargs(std::move(kwargs))));
             })
        .def(
            "add_submission",
            [](db::TableSubmissions& self, const db::SubmissionRow& row, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.addSubmission(tableExecuteOptionsFromKwargs(std::move(kwargs)), row));
            },
            py::arg("row"))
        .def(
            "load_submissions_by_user_and_problem",
            [](db::TableSubmissions& self, std::string username, std::string problem_id, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self),
                                       self.loadSubmissionsByUserAndProblem(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                            std::move(username), std::move(problem_id)));
            },
            py::arg("username"), py::arg("problem_id"))
        .def(
            "update_submission_verdict",
            [](db::TableSubmissions& self, std::string submission_id, std::string verdict_type, double score,
               py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self),
                                       self.updateSubmissionVerdict(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                    std::move(submission_id), std::move(verdict_type), score));
            },
            py::arg("submission_id"), py::arg("verdict_type"), py::arg("score"))
        .def(
            "whose_submission",
            [](db::TableSubmissions& self, std::string submission_id, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.whoseSubmission(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                            std::move(submission_id)));
            },
            py::arg("submission_id"))
        .def(
            "problem_of_submission",
            [](db::TableSubmissions& self, std::string submission_id, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.problemOfSubmission(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                                std::move(submission_id)));
            },
            py::arg("submission_id"))
        .def(
            "language_of_submission",
            [](db::TableSubmissions& self, std::string submission_id, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.languageOfSubmission(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                                 std::move(submission_id)));
            },
            py::arg("submission_id"))
        .def(
            "verdict_type_of_submission",
            [](db::TableSubmissions& self, std::string submission_id, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(
                    py::cast(self),
                    self.verdictTypeOfSubmission(tableExecuteOptionsFromKwargs(std::move(kwargs)), std::move(submission_id)));
            },
            py::arg("submission_id"))
        .def(
            "score_of_submission",
            [](db::TableSubmissions& self, std::string submission_id, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.scoreOfSubmission(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                              std::move(submission_id)));
            },
            py::arg("submission_id"))
        .def(
            "set_verdict_type",
            [](db::TableSubmissions& self, std::string submission_id, std::string verdict_type, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.setVerdictType(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                           std::move(submission_id), std::move(verdict_type)));
            },
            py::arg("submission_id"), py::arg("verdict_type"))
        .def(
            "set_score",
            [](db::TableSubmissions& self, std::string submission_id, double score, py::kwargs kwargs) -> py::object {
                return as::spawnMethod(py::cast(self), self.setScore(tableExecuteOptionsFromKwargs(std::move(kwargs)),
                                                                     std::move(submission_id), score));
            },
            py::arg("submission_id"), py::arg("score"));
}
