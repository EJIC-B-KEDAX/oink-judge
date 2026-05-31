cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::content_service

set(CONTENT_SERVICE_COMMON_PUBLIC_HEADERS
    include/oink_judge/content_service/content_manifest.h
    include/oink_judge/content_service/manifest_storage.h
    include/oink_judge/content_service/config_utils.h)
set(CONTENT_SERVICE_COMMON_SOURCES
    src/content_manifest.cpp src/manifest_storage.cpp src/config_utils.cpp)
set(CONTENT_SERVICE_CLIENT_PUBLIC_HEADERS
    include/oink_judge/content_service/client/content_client_protocol.h
    include/oink_judge/content_service/client/content_storage.h)
set(CONTENT_SERVICE_CLIENT_SOURCES src/client/content_client_protocol.cpp
                                   src/client/content_storage.cpp)
set(CONTENT_SERVICE_SERVER_PUBLIC_HEADERS
    include/oink_judge/content_service/server/content_server_protocol.h)
set(CONTENT_SERVICE_SERVER_SOURCES src/server/content_server_protocol.cpp)
set(CONTENT_SERVICE_PUBLIC_HEADERS
    ${CONTENT_SERVICE_COMMON_PUBLIC_HEADERS}
    ${CONTENT_SERVICE_CLIENT_PUBLIC_HEADERS}
    ${CONTENT_SERVICE_SERVER_PUBLIC_HEADERS})

add_library(oink_judge_content_service_headers INTERFACE
            ${CONTENT_SERVICE_PUBLIC_HEADERS})
target_include_directories(
  oink_judge_content_service_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
target_link_libraries(
  oink_judge_content_service_headers
  INTERFACE oink_judge_logger_headers oink_judge_config_headers
            oink_judge_utils_headers oink_judge_factory
            oink_judge_socket_headers)

add_library(
  oink_judge_content_service_common SHARED
  ${CONTENT_SERVICE_COMMON_SOURCES} ${CONTENT_SERVICE_COMMON_PUBLIC_HEADERS})
add_library(oink_judge::content_service_common ALIAS
            oink_judge_content_service_common)
target_link_libraries(
  oink_judge_content_service_common
  PUBLIC oink_judge_content_service_headers oink_judge::logger
         oink_judge::config oink_judge::utils oink_judge::socket)

add_library(
  oink_judge_content_service_client SHARED
  ${CONTENT_SERVICE_CLIENT_SOURCES} ${CONTENT_SERVICE_CLIENT_PUBLIC_HEADERS})
add_library(oink_judge::content_service_client ALIAS
            oink_judge_content_service_client)
target_link_libraries(oink_judge_content_service_client
                      PUBLIC oink_judge::content_service_common)

add_library(
  oink_judge_content_service_server SHARED
  ${CONTENT_SERVICE_SERVER_SOURCES} ${CONTENT_SERVICE_SERVER_PUBLIC_HEADERS})
add_library(oink_judge::content_service_server ALIAS
            oink_judge_content_service_server)
target_link_libraries(oink_judge_content_service_server
                      PUBLIC oink_judge::content_service_common)

add_executable(content_service_server src/server/server_starter.cpp)
target_link_libraries(
  content_service_server PUBLIC oink_judge::socket oink_judge::config
                                oink_judge::logger oink_judge::plugin_manager)

add_library(content_service_server_type_registration SHARED
            type_registration/server_type_registration.cpp)
target_link_libraries(content_service_server_type_registration
                      PRIVATE oink_judge::content_service_server)
target_compile_options(content_service_server_type_registration PRIVATE -fPIC)

add_custom_target(
  configure_content_service_server_plugins ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_SOURCE_DIR}/plugins/content_service/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:socket_type_registration>
          ${CMAKE_SOURCE_DIR}/plugins/content_service/
  COMMAND
    ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:content_service_server_type_registration>
    ${CMAKE_SOURCE_DIR}/plugins/content_service/
  DEPENDS content_service_server_type_registration socket_type_registration)

add_library(content_service_client_type_registration SHARED
            type_registration/client_type_registration.cpp)
target_link_libraries(content_service_client_type_registration
                      PRIVATE oink_judge::content_service_client)
target_compile_options(content_service_client_type_registration PRIVATE -fPIC)

set(PROBLEM_PACKAGE_CONVERTER_PUBLIC_HEADERS
    include/oink_judge/content_service/problem_package_converter/package_converter.hpp
    include/oink_judge/content_service/problem_package_converter/polygon_converter.h
)
set(PROBLEM_PACKAGE_CONVERTER_SOURCES
    src/problem_package_converter/main.cpp
    src/problem_package_converter/polygon_converter.cpp)

add_executable(
  problem_package_converter ${PROBLEM_PACKAGE_CONVERTER_SOURCES}
                            ${PROBLEM_PACKAGE_CONVERTER_PUBLIC_HEADERS})
target_include_directories(
  problem_package_converter
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(problem_package_converter oink_judge::factory
                      oink_judge::logger pugixml)

add_subdirectory(python_binding)
