# ------------------------------------------------------------------------------
# Dispatcher tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_dispatcher_client_config_utils_tests
                test_client_config_utils.cpp oink_judge::dispatcher)
add_module_test(oink_judge_dispatcher_invoker_tests test_invoker.cpp
                oink_judge::dispatcher)
add_module_test(oink_judge_dispatcher_testing_queue_tests
                test_testing_queue.cpp oink_judge::dispatcher)
add_module_test(oink_judge_dispatcher_submission_managers_storage_tests
                test_submission_managers_storage.cpp oink_judge::dispatcher)
add_module_test(oink_judge_dispatcher_send_submission_to_invoker_tests
                test_send_submission_to_invoker.cpp oink_judge::dispatcher)

# ------------------------------------------------------------------------------
# Copy resources
# ------------------------------------------------------------------------------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
