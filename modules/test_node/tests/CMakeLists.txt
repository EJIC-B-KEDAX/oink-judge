# ------------------------------------------------------------------------------
# Test node tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_test_node_config_utils_tests test_config_utils.cpp
                oink_judge::test_node)
add_module_test(oink_judge_test_node_verdict_base_tests test_verdict_base.cpp
                oink_judge::test_node)
add_module_test(oink_judge_test_node_verdict_utils_tests test_verdict_utils.cpp
                oink_judge::test_node)
add_module_test(oink_judge_test_node_verdict_builder_min_tests
                test_verdict_builder_min.cpp oink_judge::test_node)
add_module_test(oink_judge_test_node_verdict_builder_sum_tests
                test_verdict_builder_sum.cpp oink_judge::test_node)

# ------------------------------------------------------------------------------
# Copy resources
# ------------------------------------------------------------------------------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
