# Module: oink_judge::dispatcher

add_subdirectory(proto)

set(DISPATCHER_PUBLIC_HEADERS
    include/oink_judge/dispatcher/client/config_utils.h
    include/oink_judge/dispatcher/dispatcher_service.h
    include/oink_judge/dispatcher/invoker.h
    include/oink_judge/dispatcher/problem_submission_manager.hpp
    include/oink_judge/dispatcher/queue_manager_service.h
    include/oink_judge/dispatcher/send_submission_to_invoker.h
    include/oink_judge/dispatcher/testing_queue.h
    include/oink_judge/dispatcher/submission_managers_storage.h)

set(DISPATCHER_SOURCES
    src/client/config_utils.cpp
    src/dispatcher_service.cpp
    src/invoker.cpp
    src/queue_manager_service.cpp
    src/send_submission_to_invoker.cpp
    src/testing_queue.cpp
    src/submission_managers_storage.cpp)

# ------------------------------------------------------------------------------
# Dispatcher headers
# ------------------------------------------------------------------------------

add_library(oink_judge_dispatcher_headers INTERFACE
            ${DISPATCHER_PUBLIC_HEADERS})
target_include_directories(
  oink_judge_dispatcher_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
target_link_libraries(oink_judge_dispatcher_headers
                      INTERFACE oink_judge::boost_asio)

# ------------------------------------------------------------------------------
# Dispatcher
# ------------------------------------------------------------------------------

add_library(oink_judge_dispatcher SHARED ${DISPATCHER_SOURCES})
add_library(oink_judge::dispatcher ALIAS oink_judge_dispatcher)
target_link_libraries(
  oink_judge_dispatcher
  PUBLIC oink_judge_dispatcher_headers
         oink_judge::logger
         oink_judge::config
         oink_judge::factory
         oink_judge::content_service_client
         oink_judge::database
         oink_judge::test_node_proto
         oink_judge::dispatcher_proto
         asio-grpc::asio-grpc
         oink_judge::boost_asio)

# ------------------------------------------------------------------------------
# Dispatcher server
# ------------------------------------------------------------------------------

add_executable(dispatcher src/server_starter.cpp)
target_link_libraries(
  dispatcher PRIVATE oink_judge::dispatcher oink_judge::plugin_manager
                     oink_judge::utils::grpc)

add_subdirectory(type_registration)

# ------------------------------------------------------------------------------
# Configure dispatcher plugins
# ------------------------------------------------------------------------------

configure_service_plugins(
  configure_dispatcher_plugins dispatcher TYPE_REGISTRATIONS
  content_service_client_type_registration dispatcher_type_registration
  utils_grpc_type_registration)
add_dependencies(dispatcher configure_dispatcher_plugins)

add_subdirectory(python_binding)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
