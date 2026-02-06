cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::test_node

set(TEST_NODE_PUBLIC_HEADERS
    include/oink_judge/test_node/invoker_protocol.h
    include/oink_judge/test_node/problem_builder.hpp
    include/oink_judge/test_node/problem_table.h
    include/oink_judge/test_node/problem_tables_storage.h
	include/oink_judge/test_node/test_node_config_utils.h
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
    include/oink_judge/test_node/verdicts/default_verdict.h
    include/oink_judge/test_node/verdicts/verdict_base.h
)

set(TEST_NODE_SOURCES
    src/invoker_protocol.cpp
    src/problem_table.cpp
    src/problem_tables_storage.cpp
	src/test_node_config_utils.cpp
    src/test_node_starter.cpp
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
    src/verdicts/default_verdict.cpp
    src/verdicts/verdict_base.cpp
)

add_executable(oink_judge_test_node ${TEST_NODE_SOURCES} ${TEST_NODE_PUBLIC_HEADERS})

target_include_directories(oink_judge_test_node PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(oink_judge_test_node PUBLIC cxx_std_20)
target_link_libraries(oink_judge_test_node PUBLIC oink_judge::logger oink_judge::config oink_judge::utils oink_judge::socket oink_judge::database oink_judge::content_service)