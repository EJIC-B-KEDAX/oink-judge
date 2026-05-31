pybind11_add_module(pybind11_logger binding.cpp)
target_link_libraries(pybind11_logger PRIVATE oink_judge::logger)
target_compile_options(pybind11_logger PRIVATE -fPIC)
set_target_properties(
  pybind11_logger
  PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
             BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_target(
  install_pybind11_logger ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pybind11_logger>
          ${PATH_TO_INSTALL_PYBIND11_MODULES}
  COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen pybind11_logger --output-dir
          "${PATH_TO_INSTALL_PYBIND11_MODULES}" --ignore-all-errors
  WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
  DEPENDS pybind11_logger)
