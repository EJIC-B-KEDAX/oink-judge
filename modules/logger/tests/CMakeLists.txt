include(AddModuleTest)
add_module_test(oink_judge_logger_tests test_logger.cpp oink_judge::logger)

add_test(NAME oink_judge_python_logger_tests
         COMMAND ${Python3_EXECUTABLE} -m pytest
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_python_logger.py -v)
