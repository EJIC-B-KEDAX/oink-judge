include(AddModuleTest)
add_module_test(oink_judge_config_tests test_config.cpp oink_judge::config)
add_module_test(oink_judge_logger_config_tests test_logger_config.cpp
                oink_judge::config)
add_module_test(oink_judge_problem_config_tests test_problem_config.cpp
                oink_judge::config)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
