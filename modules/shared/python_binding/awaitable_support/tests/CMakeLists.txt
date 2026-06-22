add_oink_pybind_module(
  binder_test_module binder_test_module.cpp NO_INSTALL_ALL LINK_LIBRARIES
  oink_judge::python_binding::awaitable_support)
target_compile_features(binder_test_module PRIVATE cxx_std_20)

add_test(
  NAME oink_judge_awaitable_python_modules_prepare
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
          install_pybind11_awaitable_support install_binder_test_module)
set_tests_properties(oink_judge_awaitable_python_modules_prepare
                     PROPERTIES FIXTURES_SETUP oink_judge_awaitable_python_modules)

# ------------------------------------------------------------------------------
# Awaitable bridge tests
# ------------------------------------------------------------------------------

add_test(NAME oink_judge_awaitable_bridge_tests
         COMMAND ${Python3_EXECUTABLE} -m pytest
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_awaitable_bridge.py -v)
set_tests_properties(
  oink_judge_awaitable_bridge_tests
  PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PATH_TO_INSTALL_PYBIND11_MODULES}:$ENV{PYTHONPATH}"
    FIXTURES_REQUIRED oink_judge_awaitable_python_modules)

# ------------------------------------------------------------------------------
# Awaitable binder tests
# ------------------------------------------------------------------------------

add_test(NAME oink_judge_awaitable_binder_tests
         COMMAND ${Python3_EXECUTABLE} -m pytest
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_binder.py -v)
set_tests_properties(
  oink_judge_awaitable_binder_tests
  PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PATH_TO_INSTALL_PYBIND11_MODULES}:$ENV{PYTHONPATH}"
    FIXTURES_REQUIRED oink_judge_awaitable_python_modules)
