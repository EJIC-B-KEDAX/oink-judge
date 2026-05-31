cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::logger

set(LOGGER_PUBLIC_HEADERS include/oink_judge/logger/logger.h
                          include/oink_judge/logger/python_logger.h)

set(LOGGER_SOURCES src/logger.cpp src/python_logger.cpp)

add_library(oink_judge_logger_headers INTERFACE ${LOGGER_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_logger_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

add_library(oink_judge_logger SHARED ${LOGGER_SOURCES})
add_library(oink_judge::logger ALIAS oink_judge_logger)
target_link_libraries(
  oink_judge_logger
  PUBLIC oink_judge_logger_headers
  PRIVATE pybind11::pybind11 Python3::Python)

add_subdirectory(python_binding)
add_subdirectory(tests)
