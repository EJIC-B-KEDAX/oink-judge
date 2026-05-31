cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::python_binding::awaitable_support

set(AWAITABLE_SUPPORT_PUBLIC_HEADERS
    include/oink_judge/python_binding/awaitable_support/awaitable_binder.hpp
    include/oink_judge/python_binding/awaitable_support/awaitable_binder.inl
    include/oink_judge/python_binding/awaitable_support/awaitable_bridge.h)

set(AWAITABLE_SUPPORT_SOURCES src/awaitable_bridge.cpp)

add_library(oink_judge_awaitable_support_headers INTERFACE
            ${AWAITABLE_SUPPORT_PUBLIC_HEADERS})
target_include_directories(
  oink_judge_awaitable_support_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

add_library(oink_judge_python_binding_awaitable_support SHARED
            ${AWAITABLE_SUPPORT_SOURCES})
add_library(oink_judge::python_binding::awaitable_support ALIAS
            oink_judge_python_binding_awaitable_support)

target_compile_features(oink_judge_python_binding_awaitable_support
                        PUBLIC cxx_std_20)
target_link_libraries(
  oink_judge_python_binding_awaitable_support
  PUBLIC oink_judge_awaitable_support_headers pybind11::pybind11)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

add_subdirectory(python_binding)
