cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::content_service

set(CONTENT_SERVICE_COMMON_PUBLIC_HEADERS
    include/oink_judge/content_service/content_manifest.h
    include/oink_judge/content_service/manifest_storage.h
	include/oink_judge/content_service/content_config_utils.h
)

set(CONTENT_SERVICE_COMMON_SOURCES
    src/content_manifest.cpp
    src/manifest_storage.cpp
	src/content_config_utils.cpp
)

set(CONTENT_SERVICE_CLIENT_PUBLIC_HEADERS
	include/oink_judge/content_service/client/content_client_protocol.h
    include/oink_judge/content_service/client/content_storage.h
)

set(CONTENT_SERVICE_CLIENT_SOURCES
	src/client/content_client_protocol.cpp
    src/client/content_storage.cpp
)

set(CONTENT_SERVICE_SERVER_PUBLIC_HEADERS
    include/oink_judge/content_service/server/content_server_protocol.h
)

set(CONTENT_SERVICE_SERVER_SOURCES
    src/server/content_server_protocol.cpp
	src/server/server_starter.cpp
)

set(CONTENT_SERVICE_PUBLIC_HEADERS ${CONTENT_SERVICE_COMMON_PUBLIC_HEADERS} ${CONTENT_SERVICE_CLIENT_PUBLIC_HEADERS} ${CONTENT_SERVICE_SERVER_PUBLIC_HEADERS})

add_library(oink_judge_content_service_headers INTERFACE ${CONTENT_SERVICE_PUBLIC_HEADERS})

target_include_directories(oink_judge_content_service_headers
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(oink_judge_content_service_headers INTERFACE oink_judge_logger_headers oink_judge_config_headers oink_judge_utils_headers oink_judge_factory oink_judge_socket_headers)

add_library(oink_judge_content_service_common SHARED ${CONTENT_SERVICE_COMMON_SOURCES} ${CONTENT_SERVICE_COMMON_PUBLIC_HEADERS})
add_library(oink_judge::content_service_common ALIAS oink_judge_content_service_common)

target_compile_features(oink_judge_content_service_common PUBLIC cxx_std_20)
target_link_libraries(oink_judge_content_service_common PUBLIC oink_judge_content_service_headers oink_judge::logger oink_judge::config oink_judge::utils oink_judge::socket)


add_library(oink_judge_content_service_client SHARED ${CONTENT_SERVICE_CLIENT_SOURCES} ${CONTENT_SERVICE_CLIENT_PUBLIC_HEADERS})
add_library(oink_judge::content_service_client ALIAS oink_judge_content_service_client)

target_link_libraries(oink_judge_content_service_client PUBLIC oink_judge::content_service_common)


add_executable(oink_judge_content_service_server ${CONTENT_SERVICE_SERVER_SOURCES} ${CONTENT_SERVICE_SERVER_PUBLIC_HEADERS})
target_link_libraries(oink_judge_content_service_server PUBLIC oink_judge::content_service_common)

set(PROBLEM_PACKAGE_CONVERTER_PUBLIC_HEADERS
	include/oink_judge/content_service/problem_package_converter/package_converter.hpp
	include/oink_judge/content_service/problem_package_converter/polygon_converter.h
)

set(PROBLEM_PACKAGE_CONVERTER_SOURCES
	src/problem_package_converter/main.cpp	
	src/problem_package_converter/polygon_converter.cpp
)

add_executable(problem_package_converter ${PROBLEM_PACKAGE_CONVERTER_SOURCES} ${PROBLEM_PACKAGE_CONVERTER_PUBLIC_HEADERS})

target_include_directories(problem_package_converter PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(problem_package_converter oink_judge::factory oink_judge::logger pugixml)
