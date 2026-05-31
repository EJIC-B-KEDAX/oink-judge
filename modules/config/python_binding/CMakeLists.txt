pybind11_add_module(pybind11_config binding.cpp)
target_link_libraries(pybind11_config PRIVATE oink_judge::config)
target_compile_options(pybind11_config PRIVATE -fPIC)
set_target_properties(
  pybind11_config
  PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
             BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_target(
  install_pybind11_config ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pybind11_config>
          ${PATH_TO_INSTALL_PYBIND11_MODULES}
  COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen pybind11_config --output-dir
          "${PATH_TO_INSTALL_PYBIND11_MODULES}" --ignore-all-errors
  WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
  DEPENDS pybind11_config)
