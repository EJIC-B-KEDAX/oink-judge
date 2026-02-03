cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::config
# This CMakeLists provides a reusable `oink_judge::config` target with public
# headers under `include/oink/config` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
set(CONFIG_PUBLIC_HEADERS
	include/oink_judge/config/config.h
    include/oink_judge/config/server_config_utils.h
	include/oink_judge/config/problem_config_utils.h
	include/oink_judge/config/logger_config_utils.h
)

set(CONFIG_SOURCES
	src/config.cpp
    src/server_config_utils.cpp
	src/problem_config_utils.cpp
	src/logger_config_utils.cpp
)

add_library(oink_judge_config STATIC ${CONFIG_SOURCES} ${CONFIG_PUBLIC_HEADERS})
add_library(oink_judge::config ALIAS oink_judge_config)

target_include_directories(oink_judge_config
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_config PUBLIC cxx_std_20)

find_package(PugiXML REQUIRED)
target_link_libraries(oink_judge_config PUBLIC pugixml oink_judge::logger)

option(OINK_CONFIG_INSTALL "Install oink_judge::config target" ON)
if(OINK_CONFIG_INSTALL)
	install(TARGETS oink_judge_config
        EXPORT oinkJudgeTargets
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include
	)

	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
		DESTINATION include
		FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
	)

	install(EXPORT oinkJudgeTargets
		NAMESPACE oink_judge::
		FILE oinkJudgeTargets-config.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_config.cpp)
    add_executable(oink_judge_config_tests tests/test_config.cpp)
    target_link_libraries(oink_judge_config_tests PRIVATE oink_judge::config)
    add_test(NAME oink_judge_config_tests COMMAND oink_judge_config_tests)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_server_config.cpp)
    add_executable(oink_judge_server_config_tests tests/test_server_config.cpp)
    target_link_libraries(oink_judge_server_config_tests PRIVATE oink_judge::config)
    add_test(NAME oink_judge_server_config_tests COMMAND oink_judge_server_config_tests)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_problem_config.cpp)
    add_executable(oink_judge_problem_config_tests tests/test_problem_config.cpp)
    target_link_libraries(oink_judge_problem_config_tests PRIVATE oink_judge::config)
    add_test(NAME oink_judge_problem_config_tests COMMAND oink_judge_problem_config_tests)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_logger_config.cpp)
    add_executable(oink_judge_logger_config_tests tests/test_logger_config.cpp)
    target_link_libraries(oink_judge_logger_config_tests PRIVATE oink_judge::config)
    add_test(NAME oink_judge_logger_config_tests COMMAND oink_judge_logger_config_tests)
endif()
