cmake_minimum_required(VERSION 3.28)
project(oink-judge)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FetchContent)

# ------------------------------------------------------------------------------
# Find packages
# ------------------------------------------------------------------------------

find_package(PugiXML REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED IMPORTED_TARGET libpqxx)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
find_package(libzip REQUIRED)
find_package(Boost 1.56 REQUIRED)

# ------------------------------------------------------------------------------
# External dependencies
# ------------------------------------------------------------------------------

add_subdirectory(external/pybind11)

if(BUILD_TESTING)
  add_subdirectory(external/googletest)
endif()

# ------------------------------------------------------------------------------
# Python binding
# ------------------------------------------------------------------------------

find_package(
  Python3
  COMPONENTS Interpreter Development
  REQUIRED)
set(PATH_TO_INSTALL_PYBIND11_MODULES ${Python3_SITEARCH}/oink_judge)
get_filename_component(OINK_JUDGE_PYTHON_PROTO_OUT_DIR
                       ${PATH_TO_INSTALL_PYBIND11_MODULES} DIRECTORY)
set(OINK_JUDGE_PLUGINS_DIR ${CMAKE_BINARY_DIR}/plugins)

file(RELATIVE_PATH RPATH_FOR_PYBIND11_MODULES
     "${PATH_TO_INSTALL_PYBIND11_MODULES}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_custom_target(
  initialise_directory_for_pybind11_modules ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${PATH_TO_INSTALL_PYBIND11_MODULES})
add_custom_target(
  initialise_plugins_directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory
                                           ${OINK_JUDGE_PLUGINS_DIR})

message(
  STATUS "PATH_TO_INSTALL_PYBIND11_MODULES: ${PATH_TO_INSTALL_PYBIND11_MODULES}"
)
message(STATUS "OINK_JUDGE_PLUGINS_DIR: ${OINK_JUDGE_PLUGINS_DIR}")
message(STATUS "RPATH_FOR_PYBIND11_MODULES: ${RPATH_FOR_PYBIND11_MODULES}")

execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import grpc_tools, mypy_protobuf"
  RESULT_VARIABLE OINK_JUDGE_GRPC_TOOLS_AVAILABLE
  ERROR_QUIET OUTPUT_QUIET)

include(AddPybindModule)
include(ConfigurePlugins)

if(BUILD_TESTING)
  include(GoogleTest)
  include(AddModuleTest)
endif()

# ------------------------------------------------------------------------------
# gRPC
# ------------------------------------------------------------------------------

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
include(AddGrpcProto)

# ------------------------------------------------------------------------------
# asio-grpc
# ------------------------------------------------------------------------------

FetchContent_Declare(
  asio-grpc
  GIT_REPOSITORY https://github.com/Tradias/asio-grpc
  GIT_TAG v3.7.0)
FetchContent_MakeAvailable(asio-grpc)

# ------------------------------------------------------------------------------
# tl-expected
# ------------------------------------------------------------------------------

FetchContent_Declare(
  tl-expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected
  GIT_TAG v1.3.1)
FetchContent_MakeAvailable(tl-expected)

add_subdirectory(modules/logger)
add_subdirectory(modules/shared)
add_subdirectory(modules/config)
add_subdirectory(modules/plugin_manager)
add_subdirectory(modules/database)
add_subdirectory(modules/content_service)
add_subdirectory(modules/dispatcher)
add_subdirectory(modules/test_node)
add_subdirectory(app)
