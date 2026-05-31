pybind11_add_module(pybind11_socket binding.cpp)
target_link_libraries(pybind11_socket PRIVATE oink_judge::socket)
target_compile_options(pybind11_socket PRIVATE -fPIC)
set_target_properties(
  pybind11_socket
  PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
             BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_target(
  install_pybind11_socket ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pybind11_socket>
          ${PATH_TO_INSTALL_PYBIND11_MODULES}
  COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen pybind11_socket --output-dir
          "${PATH_TO_INSTALL_PYBIND11_MODULES}" --ignore-all-errors
  WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
  DEPENDS pybind11_socket)
