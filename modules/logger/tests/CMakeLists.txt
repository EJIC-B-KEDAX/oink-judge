# ------------------------------------------------------------------------------
# Logger tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_logger_tests test_logger.cpp oink_judge::logger)

# ------------------------------------------------------------------------------
# Python logger tests
# ------------------------------------------------------------------------------

add_test(NAME oink_judge_python_logger_tests_prepare
         COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
                 install_pybind11_logger)
set_tests_properties(oink_judge_python_logger_tests_prepare
                     PROPERTIES FIXTURES_SETUP oink_judge_python_logger_modules)

add_test(NAME oink_judge_python_logger_tests
         COMMAND ${Python3_EXECUTABLE} -m pytest
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_python_logger.py -v)
set_tests_properties(
  oink_judge_python_logger_tests
  PROPERTIES ENVIRONMENT
             "PYTHONPATH=${PATH_TO_INSTALL_PYBIND11_MODULES}:$ENV{PYTHONPATH}"
             FIXTURES_REQUIRED oink_judge_python_logger_modules)
