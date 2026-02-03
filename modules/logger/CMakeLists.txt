cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::logger
# This CMakeLists provides a reusable `oink_judge::logger` target with public
# headers under `include/oink/logger` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
set(LOGGER_PUBLIC_HEADERS
	include/oink_judge/logger/logger.h
)

set(LOGGER_SOURCES
	src/logger.cpp
)

add_library(oink_judge_logger STATIC ${LOGGER_SOURCES} ${LOGGER_PUBLIC_HEADERS})
add_library(oink_judge::logger ALIAS oink_judge_logger)
target_include_directories(oink_judge_logger
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_logger PUBLIC cxx_std_20)

option(OINK_LOGGER_INSTALL "Install oink_judge::logger target" ON)
if(OINK_LOGGER_INSTALL)
	install(TARGETS oink_judge_logger
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
		FILE oinkJudgeTargets-logger.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_logger.cpp)
    add_executable(oink_judge_logger_tests tests/test_logger.cpp)
    target_link_libraries(oink_judge_logger_tests PRIVATE oink_judge::logger)
    add_test(NAME oink_judge_logger_tests COMMAND oink_judge_logger_tests)
endif()
