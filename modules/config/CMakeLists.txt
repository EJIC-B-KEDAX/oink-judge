cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::config

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

add_library(oink_judge_config_headers INTERFACE ${CONFIG_PUBLIC_HEADERS})

target_include_directories(oink_judge_config_headers
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(oink_judge_config_headers INTERFACE oink_judge_logger_headers)

add_library(oink_judge_config SHARED ${CONFIG_SOURCES})
add_library(oink_judge::config ALIAS oink_judge_config)

target_compile_features(oink_judge_config PUBLIC cxx_std_20)

find_package(PugiXML REQUIRED)
target_link_libraries(oink_judge_config PUBLIC oink_judge_config_headers pugixml oink_judge::logger)


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
