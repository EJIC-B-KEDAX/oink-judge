cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::utils
# This CMakeLists provides a reusable `oink_judge::utils` target with public
# headers under `include/oink/utils` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
set(CONFIG_PUBLIC_HEADERS
	include/oink_judge/utils/async.h
    include/oink_judge/utils/crypto.h
    include/oink_judge/utils/filesystem.h
)

set(CONFIG_SOURCES
    src/async.cpp
    src/crypto.cpp
    src/filesystem.cpp
)

add_library(oink_judge_utils STATIC ${CONFIG_SOURCES} ${CONFIG_PUBLIC_HEADERS})
add_library(oink_judge::utils ALIAS oink_judge_utils)

target_include_directories(oink_judge_utils
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_utils PUBLIC cxx_std_20)

target_link_libraries(oink_judge_utils PUBLIC zip sodium oink_judge::logger)

option(OINK_UTILS_INSTALL "Install oink_judge::utils target" ON)
if(OINK_UTILS_INSTALL)
	install(TARGETS oink_judge_utils
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
		FILE oinkJudgeTargets-utils.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()
