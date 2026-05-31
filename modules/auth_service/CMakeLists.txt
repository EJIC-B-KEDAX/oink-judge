cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::auth_service

set(AUTH_SERVICE_PUBLIC_HEADERS
    include/oink_judge/auth_service/auth_manager.h
    include/oink_judge/auth_service/handle_request.h
    include/oink_judge/auth_service/protocol_with_fastapi.h
    include/oink_judge/auth_service/session.h
    include/oink_judge/auth_service/table_sessions.h
    include/oink_judge/auth_service/table_users.h)

set(AUTH_SERVICE_SOURCES
    src/auth_manager.cpp src/handle_request.cpp src/protocol_with_fastapi.cpp
    src/session.cpp src/table_sessions.cpp src/table_users.cpp)

add_library(oink_judge_auth_service_headers INTERFACE
            ${AUTH_SERVICE_PUBLIC_HEADERS})
target_include_directories(
  oink_judge_auth_service_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
target_link_libraries(
  oink_judge_auth_service_headers
  INTERFACE oink_judge_logger_headers oink_judge_config_headers
            oink_judge_utils_headers oink_judge_socket_headers
            oink_judge_database_headers oink_judge_plugin_manager_headers)

add_library(oink_judge_auth_service SHARED ${AUTH_SERVICE_SOURCES})
add_library(oink_judge::auth_service ALIAS oink_judge_auth_service)

target_compile_features(oink_judge_auth_service PUBLIC cxx_std_20)
target_link_libraries(
  oink_judge_auth_service
  PUBLIC oink_judge_auth_service_headers
         oink_judge::logger
         oink_judge::config
         oink_judge::utils
         oink_judge::socket
         oink_judge::database
         oink_judge::plugin_manager)

add_library(auth_service_types_registration SHARED
            type_registration/type_registration.cpp)
target_link_libraries(auth_service_types_registration
                      PRIVATE oink_judge::auth_service)
target_compile_options(auth_service_types_registration PRIVATE -fPIC)

add_executable(auth_service src/server_starter.cpp)
set_target_properties(auth_service PROPERTIES ENABLE_EXPORTS ON)

target_link_libraries(auth_service PUBLIC oink_judge::auth_service)

add_custom_target(
  configure_auth_service_plugins ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_SOURCE_DIR}/plugins/auth_service/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:socket_type_registration>
          ${CMAKE_SOURCE_DIR}/plugins/auth_service/
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:auth_service_types_registration>
    ${CMAKE_SOURCE_DIR}/plugins/auth_service/
  DEPENDS socket_type_registration auth_service_types_registration)
