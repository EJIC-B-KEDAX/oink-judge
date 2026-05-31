pybind11_add_module(pybind11_content_service binding.cpp)
target_link_libraries(
  pybind11_content_service PRIVATE oink_judge::content_service_client
                                   oink_judge::python_binding)
target_compile_options(pybind11_content_service PRIVATE -fPIC)
set_target_properties(
  pybind11_content_service
  PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
             BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_target(
  install_pybind11_content_service ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pybind11_content_service>
          ${PATH_TO_INSTALL_PYBIND11_MODULES}
  COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen pybind11_content_service
          --output-dir "${PATH_TO_INSTALL_PYBIND11_MODULES}" --ignore-all-errors
  WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
  DEPENDS pybind11_content_service)
