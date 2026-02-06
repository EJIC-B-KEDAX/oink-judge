cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::content_service
# This CMakeLists provides a reusable `oink_judge::content_service` target with public
# headers under `include/oink/content_service` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
set(CONTENT_SERVICE_PUBLIC_HEADERS
	include/oink_judge/content_service/content_client_protocol.h
    include/oink_judge/content_service/content_manifest.h
    include/oink_judge/content_service/content_server_protocol.h
    include/oink_judge/content_service/content_storage.h
    include/oink_judge/content_service/manifest_storage.h
)

set(CONTENT_SERVICE_SOURCES
	src/content_client_protocol.cpp
    src/content_manifest.cpp
    src/content_server_protocol.cpp
    src/content_storage.cpp
    src/manifest_storage.cpp
)

add_library(oink_judge_content_service STATIC ${CONTENT_SERVICE_SOURCES} ${CONTENT_SERVICE_PUBLIC_HEADERS})
add_library(oink_judge::content_service ALIAS oink_judge_content_service)

target_include_directories(oink_judge_content_service
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_content_service PUBLIC cxx_std_20)
target_link_libraries(oink_judge_content_service PUBLIC oink_judge::logger oink_judge::config oink_judge::utils oink_judge::factory oink_judge::socket)

option(OINK_CONTENT_SERVICE_INSTALL "Install oink_judge::content_service target" ON)
if(OINK_CONTENT_SERVICE_INSTALL)
	install(TARGETS oink_judge_content_service
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
		FILE oinkJudgeTargets-content_service.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()

set(PROBLEM_PACKAGE_CONVERTER_PUBLIC_HEADERS
	include/oink_judge/content_service/problem_package_converter/package_converter.hpp
	include/oink_judge/content_service/problem_package_converter/polygon_converter.h
)

set(PROBLEM_PACKAGE_CONVERTER_SOURCES
	src/problem_package_converter/polygon_converter.cpp
)

add_executable(problem_package_converter ${PROBLEM_PACKAGE_CONVERTER_SOURCES} ${PROBLEM_PACKAGE_CONVERTER_PUBLIC_HEADERS}
	src/problem_package_converter/main.cpp)

target_include_directories(problem_package_converter PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(problem_package_converter oink_judge::factory oink_judge::config oink_judge::logger)
