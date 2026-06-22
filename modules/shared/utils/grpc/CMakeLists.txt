# Module: oink_judge::utils::grpc

set(UTILS_GRPC_PUBLIC_HEADERS
    include/oink_judge/utils/grpc/factories.hpp
    include/oink_judge/utils/grpc/base_types.h
    include/oink_judge/utils/grpc/auth_metadata_plugin.h
    include/oink_judge/utils/grpc/auth_server_interceptor.h
    include/oink_judge/utils/grpc/config_utils.h)

set(UTILS_GRPC_SOURCES src/base_types.cpp src/auth_metadata_plugin.cpp
                       src/auth_server_interceptor.cpp src/config_utils.cpp)

# ------------------------------------------------------------------------------
# Utils grpc headers
# ------------------------------------------------------------------------------

add_library(oink_judge_utils_grpc_headers INTERFACE
            ${UTILS_GRPC_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_utils_grpc_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(
  oink_judge_utils_grpc_headers
  INTERFACE oink_judge_logger_headers oink_judge_factory
            oink_judge_utils_headers oink_judge_config_headers)

# ------------------------------------------------------------------------------
# Utils grpc
# ------------------------------------------------------------------------------

add_library(oink_judge_utils_grpc SHARED ${UTILS_GRPC_SOURCES})
add_library(oink_judge::utils::grpc ALIAS oink_judge_utils_grpc)

target_link_libraries(
  oink_judge_utils_grpc
  PUBLIC oink_judge_utils_grpc_headers oink_judge::logger oink_judge::utils
         oink_judge::config gRPC::grpc++)

add_subdirectory(type_registration)
add_subdirectory(python_binding)
