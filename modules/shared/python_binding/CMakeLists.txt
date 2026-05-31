cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::python_binding

set(PYTHON_BINDING_PUBLIC_HEADERS
    include/oink_judge/python_binding/json_type_caster.hpp)

add_library(oink_judge_python_binding INTERFACE
            ${PYTHON_BINDING_PUBLIC_HEADERS})
add_library(oink_judge::python_binding ALIAS oink_judge_python_binding)

target_include_directories(
  oink_judge_python_binding
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_compile_features(oink_judge_python_binding INTERFACE cxx_std_20)

add_subdirectory(awaitable_support)
