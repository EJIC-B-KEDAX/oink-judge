# ------------------------------------------------------------------------------
# Database tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_database_config_utils_tests test_config_utils.cpp
                oink_judge::database)
add_module_test(oink_judge_database_database_executor_tests test_database_executor.cpp
                oink_judge::database)
add_module_test(oink_judge_database_execute_options_tests test_execute_options.cpp
                oink_judge::database)
add_module_test(oink_judge_database_statements_tests test_statements.cpp
                oink_judge::database)
add_module_test(oink_judge_database_statement_action_log_tests
                test_statement_action_log.cpp oink_judge::database)
add_module_test(oink_judge_database_table_execute_options_tests
                test_table_execute_options.cpp oink_judge::database)
add_module_test(oink_judge_database_table_submissions_tests test_table_submissions.cpp
                oink_judge::database)
add_module_test(oink_judge_database_connection_pool_tests test_connection_pool.cpp
                oink_judge::database)

# ------------------------------------------------------------------------------
# Database integration tests
# ------------------------------------------------------------------------------

add_executable(oink_judge_database_integration_tests
               test_connection_pool_integration.cpp)
target_link_libraries(oink_judge_database_integration_tests
                      PRIVATE oink_judge::database GTest::gtest_main)
gtest_discover_tests(
  oink_judge_database_integration_tests
  PROPERTIES ENVIRONMENT
             "OINK_JUDGE_TEST_CONFIG_DIR=${CMAKE_CURRENT_BINARY_DIR}/resources"
             DISABLED)

# ------------------------------------------------------------------------------
# Database python integration tests
# ------------------------------------------------------------------------------

add_test(
  NAME oink_judge_database_python_modules_prepare
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
          install_pybind11_database install_pybind11_awaitable_support
          install_pybind11_config)
set_tests_properties(oink_judge_database_python_modules_prepare
                     PROPERTIES FIXTURES_SETUP oink_judge_database_python_modules)

add_test(
  NAME oink_judge_database_python_integration_test
  COMMAND ${Python3_EXECUTABLE} -m pytest
          ${CMAKE_CURRENT_SOURCE_DIR}/test_database_python_integration.py -v)
set_tests_properties(
  oink_judge_database_python_integration_test
  PROPERTIES
    ENVIRONMENT
    "PYTHONPATH=${PATH_TO_INSTALL_PYBIND11_MODULES}:$ENV{PYTHONPATH};OINK_JUDGE_TEST_CONFIG_DIR=${CMAKE_CURRENT_BINARY_DIR}/resources"
    WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
    FIXTURES_REQUIRED
    oink_judge_database_python_modules
    TIMEOUT
    30)

# ------------------------------------------------------------------------------
# Database python exceptions test
# ------------------------------------------------------------------------------

add_test(
  NAME oink_judge_database_python_exceptions_test
  COMMAND ${Python3_EXECUTABLE} -m pytest
          ${CMAKE_CURRENT_SOURCE_DIR}/test_database_python_exceptions.py -v)
set_tests_properties(
  oink_judge_database_python_exceptions_test
  PROPERTIES
    ENVIRONMENT
    "PYTHONPATH=${PATH_TO_INSTALL_PYBIND11_MODULES}:$ENV{PYTHONPATH};OINK_JUDGE_TEST_CONFIG_DIR=${CMAKE_CURRENT_BINARY_DIR}/resources"
    WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
    FIXTURES_REQUIRED
    oink_judge_database_python_modules
    TIMEOUT
    30)

# ------------------------------------------------------------------------------
# Copy resources
# ------------------------------------------------------------------------------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
