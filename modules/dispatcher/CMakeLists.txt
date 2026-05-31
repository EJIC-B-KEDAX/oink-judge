cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::dispatcher

set(DISPATCHER_PUBLIC_HEADERS
    include/oink_judge/dispatcher/invoker.h
    include/oink_judge/dispatcher/problem_submission_manager.hpp
    include/oink_judge/dispatcher/protocol_with_fastapi.h
    include/oink_judge/dispatcher/protocol_with_invoker.h
    include/oink_judge/dispatcher/send_submission_to_invoker.h
    include/oink_judge/dispatcher/testing_queue.h)
set(DISPATCHER_SOURCES
    src/invoker.cpp src/protocol_with_fastapi.cpp src/protocol_with_invoker.cpp
    src/send_submission_to_invoker.cpp src/testing_queue.cpp)

add_library(oink_judge_dispatcher_headers INTERFACE
            ${DISPATCHER_PUBLIC_HEADERS})
target_include_directories(
  oink_judge_dispatcher_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

add_library(oink_judge_dispatcher SHARED ${DISPATCHER_SOURCES})
add_library(oink_judge::dispatcher ALIAS oink_judge_dispatcher)
target_link_libraries(
  oink_judge_dispatcher
  PUBLIC oink_judge_dispatcher_headers
         oink_judge::logger
         oink_judge::config
         oink_judge::factory
         oink_judge::socket
         oink_judge::content_service_client
         oink_judge::database)

add_executable(dispatcher src/server_starter.cpp)
target_link_libraries(
  dispatcher PUBLIC oink_judge::logger oink_judge::config oink_judge::socket
                    oink_judge::plugin_manager)

add_library(dispatcher_type_registration SHARED
            type_registration/type_registration.cpp)
target_link_libraries(dispatcher_type_registration
                      PRIVATE oink_judge::dispatcher)
target_compile_options(dispatcher_type_registration PRIVATE -fPIC)

add_custom_target(
  configure_dispatcher_plugins ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_SOURCE_DIR}/plugins/dispatcher/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:socket_type_registration>
          ${CMAKE_SOURCE_DIR}/plugins/dispatcher/
  COMMAND
    ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:content_service_client_type_registration>
    ${CMAKE_SOURCE_DIR}/plugins/dispatcher/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dispatcher_type_registration>
          ${CMAKE_SOURCE_DIR}/plugins/dispatcher/
  DEPENDS socket_type_registration content_service_client_type_registration
          dispatcher_type_registration)
