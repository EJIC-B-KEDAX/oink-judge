cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::logger

set(LOGGER_PUBLIC_HEADERS
	include/oink_judge/logger/logger.h
	include/oink_judge/logger/logger.inl
)

set(LOGGER_SOURCES
	src/logger.cpp
)

add_library(oink_judge_logger_headers INTERFACE ${LOGGER_PUBLIC_HEADERS})

target_include_directories(oink_judge_logger_headers
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

add_library(oink_judge_logger SHARED ${LOGGER_SOURCES})
add_library(oink_judge::logger ALIAS oink_judge_logger)

target_compile_features(oink_judge_logger PUBLIC cxx_std_20)
target_link_libraries(oink_judge_logger PUBLIC oink_judge_logger_headers)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_logger.cpp)
    add_executable(oink_judge_logger_tests tests/test_logger.cpp)
    target_link_libraries(oink_judge_logger_tests PRIVATE oink_judge::logger)
    add_test(NAME oink_judge_logger_tests COMMAND oink_judge_logger_tests)
endif()
