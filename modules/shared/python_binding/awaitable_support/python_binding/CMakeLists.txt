pybind11_add_module(pybind11_awaitable_support binding.cpp)
target_link_libraries(pybind11_awaitable_support
                      PRIVATE oink_judge::python_binding::awaitable_support)
target_compile_options(pybind11_awaitable_support PRIVATE -fPIC)
set_target_properties(
  pybind11_awaitable_support
  PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
             BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_target(
  install_pybind11_awaitable_support ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pybind11_awaitable_support>
          ${PATH_TO_INSTALL_PYBIND11_MODULES}
  COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen pybind11_awaitable_support
          --output-dir "${PATH_TO_INSTALL_PYBIND11_MODULES}" --ignore-all-errors
  WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
  DEPENDS pybind11_awaitable_support)
