# Tests for oink_judge::python_binding::awaitable_support
#
# Tests are pure Python / pytest files.  No pybind11::embed needed.
#
# test_awaitable_bridge.py  — uses the installed
# oink_judge.pybind11_awaitable_support test_binder.py            — uses
# binder_test_module (C++ test helpers built below)

# ============================================================================
# binder_test_module — C++ pybind11 extension that exposes test awaitables (free
# functions and Counter class) for use by test_binder.py.
# ============================================================================
pybind11_add_module(binder_test_module binder_test_module.cpp)
target_compile_features(binder_test_module PRIVATE cxx_std_20)
target_link_libraries(binder_test_module
                      PRIVATE oink_judge::python_binding::awaitable_support)
set_target_properties(
  binder_test_module
  PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
             BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_target(
  install_binder_test_module ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:binder_test_module>
          ${PATH_TO_INSTALL_PYBIND11_MODULES}
  COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen binder_test_module
          --output-dir "${PATH_TO_INSTALL_PYBIND11_MODULES}" --ignore-all-errors
  WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
  DEPENDS binder_test_module)

# ============================================================================
# oink_judge_awaitable_bridge_tests
# ============================================================================
add_test(NAME oink_judge_awaitable_bridge_tests
         COMMAND ${Python3_EXECUTABLE} -m pytest
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_awaitable_bridge.py -v)

# ============================================================================
# oink_judge_awaitable_binder_tests
#
# PYTHONPATH is set so that binder_test_module.cpython-*.so (built into
# CMAKE_CURRENT_BINARY_DIR) is importable.
# ============================================================================
add_test(NAME oink_judge_awaitable_binder_tests
         COMMAND ${Python3_EXECUTABLE} -m pytest
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_binder.py -v)
