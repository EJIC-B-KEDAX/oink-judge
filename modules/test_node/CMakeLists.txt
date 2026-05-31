cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::test_node

set(TEST_NODE_PUBLIC_HEADERS
    include/oink_judge/test_node/config_utils.h
    include/oink_judge/test_node/invoker_protocol.h
    include/oink_judge/test_node/problem_builder.hpp
    include/oink_judge/test_node/problem_table.h
    include/oink_judge/test_node/problem_tables_storage.h
    include/oink_judge/test_node/test_storage.h
    include/oink_judge/test_node/test.hpp
    include/oink_judge/test_node/verdict_aggregator.hpp
    include/oink_judge/test_node/verdict_builder.hpp
    include/oink_judge/test_node/verdict_type.hpp
    include/oink_judge/test_node/verdict_utils.h
    include/oink_judge/test_node/verdict.hpp
    include/oink_judge/test_node/problem_builders/default_problem_builder.h
    include/oink_judge/test_node/problem_builders/enable_get_test_by_name_impl.h
    include/oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp
    include/oink_judge/test_node/tests/compilation_test.h
    include/oink_judge/test_node/tests/single_test.h
    include/oink_judge/test_node/tests/sync_result_test.h
    include/oink_judge/test_node/tests/testset.h
    include/oink_judge/test_node/verdict_aggregators/aggregate_current.h
    include/oink_judge/test_node/verdict_aggregators/aggregate_max.h
    include/oink_judge/test_node/verdict_builders/verdict_builder_min.h
    include/oink_judge/test_node/verdict_builders/verdict_builder_sum.h
    include/oink_judge/test_node/verdicts/default_verdict.h
    include/oink_judge/test_node/verdicts/verdict_base.h)
set(TEST_NODE_SOURCES
    src/config_utils.cpp
    src/invoker_protocol.cpp
    src/problem_table.cpp
    src/problem_tables_storage.cpp
    src/test_storage.cpp
    src/verdict_utils.cpp
    src/problem_builders/default_problem_builder.cpp
    src/problem_builders/enable_get_test_by_name_impl.cpp
    src/tests/compilation_test.cpp
    src/tests/single_test.cpp
    src/tests/sync_result_test.cpp
    src/tests/testset.cpp
    src/verdict_aggregators/aggregate_current.cpp
    src/verdict_aggregators/aggregate_max.cpp
    src/verdict_builders/verdict_builder_min.cpp
    src/verdict_builders/verdict_builder_sum.cpp
    src/verdicts/default_verdict.cpp
    src/verdicts/verdict_base.cpp)

add_library(oink_judge_test_node_headers INTERFACE ${TEST_NODE_PUBLIC_HEADERS})
target_include_directories(
  oink_judge_test_node_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
target_link_libraries(
  oink_judge_test_node_headers
  INTERFACE oink_judge_logger_headers oink_judge_config_headers
            oink_judge_utils_headers oink_judge_socket_headers
            oink_judge_database_headers oink_judge_content_service_headers)

add_library(oink_judge_test_node SHARED ${TEST_NODE_SOURCES})
add_library(oink_judge::test_node ALIAS oink_judge_test_node)
target_link_libraries(
  oink_judge_test_node
  PUBLIC oink_judge_test_node_headers
         oink_judge::logger
         oink_judge::config
         oink_judge::utils
         oink_judge::socket
         oink_judge::database
         oink_judge::content_service_client)

add_executable(test_node src/test_node_starter.cpp)
target_link_libraries(test_node PUBLIC oink_judge::test_node
                                       oink_judge::plugin_manager)

add_library(test_node_type_registration SHARED
            type_registration/type_registration.cpp)
target_link_libraries(test_node_type_registration PRIVATE oink_judge::test_node)
target_compile_options(test_node_type_registration PRIVATE -fPIC)

add_custom_target(
  configure_test_node_plugins ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_SOURCE_DIR}/plugins/test_node/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:socket_type_registration>
          ${CMAKE_SOURCE_DIR}/plugins/test_node/
  COMMAND
    ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:content_service_client_type_registration>
    ${CMAKE_SOURCE_DIR}/plugins/test_node/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test_node_type_registration>
          ${CMAKE_SOURCE_DIR}/plugins/test_node/
  DEPENDS socket_type_registration content_service_client_type_registration
          test_node_type_registration)
