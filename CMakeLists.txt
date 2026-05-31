cmake_minimum_required(VERSION 3.28)
project(oink-judge)
include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(external/pybind11)

find_package(
  Python3
  COMPONENTS Interpreter Development
  REQUIRED)
set(PATH_TO_INSTALL_PYBIND11_MODULES ${Python3_SITEARCH}/oink_judge)
add_custom_target(
  initialise_directory_for_pybind11_modules ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${PATH_TO_INSTALL_PYBIND11_MODULES})
message("PATH_TO_INSTALL_PYBIND11_MODULES: ${PATH_TO_INSTALL_PYBIND11_MODULES}")
set(RPATH_FOR_PYBIND11_MODULES "../../../../../build/lib")

add_subdirectory(external/googletest)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(modules/logger)
add_subdirectory(modules/shared)
add_subdirectory(modules/config)
add_subdirectory(modules/plugin_manager)
add_subdirectory(modules/database)
add_subdirectory(modules/socket)
add_subdirectory(modules/content_service)
add_subdirectory(modules/dispatcher)
add_subdirectory(modules/auth_service)
add_subdirectory(modules/test_node)
