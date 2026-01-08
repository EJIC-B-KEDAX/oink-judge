cmake_minimum_required(VERSION 3.28)
project(oink-judge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

include(cmake/factory.cmake)
include(cmake/config.cmake)
include(cmake/socket.cmake)
include(cmake/database.cmake)
include(cmake/auth.cmake)
include(cmake/data_sender.cmake)
include(cmake/dispatcher.cmake)
include(cmake/test_node.cmake)
include(cmake/problem_package_converter.cmake)
