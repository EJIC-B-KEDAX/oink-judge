function(add_oink_pybind_module MODULE_NAME SOURCE_FILE)
  cmake_parse_arguments(ARG "NO_INSTALL_ALL" "" "LINK_LIBRARIES" ${ARGN})

  pybind11_add_module(${MODULE_NAME} ${SOURCE_FILE})
  target_link_libraries(${MODULE_NAME} PRIVATE ${ARG_LINK_LIBRARIES})
  set_target_properties(
    ${MODULE_NAME}
    PROPERTIES INSTALL_RPATH "$ORIGIN/${RPATH_FOR_PYBIND11_MODULES}"
               BUILD_WITH_INSTALL_RPATH TRUE)

  if(NOT ARG_NO_INSTALL_ALL)
    add_custom_target(
      install_${MODULE_NAME} ALL
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_NAME}>
              ${PATH_TO_INSTALL_PYBIND11_MODULES}
      COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen ${MODULE_NAME}
              --output-dir "${PATH_TO_INSTALL_PYBIND11_MODULES}"
              --ignore-all-errors
      WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
      DEPENDS initialise_directory_for_pybind11_modules ${MODULE_NAME}
      VERBATIM)
  else()
    add_custom_target(
      install_${MODULE_NAME}
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_NAME}>
              ${PATH_TO_INSTALL_PYBIND11_MODULES}
      COMMAND ${Python3_EXECUTABLE} -m pybind11_stubgen ${MODULE_NAME}
              --output-dir "${PATH_TO_INSTALL_PYBIND11_MODULES}"
              --ignore-all-errors
      WORKING_DIRECTORY ${PATH_TO_INSTALL_PYBIND11_MODULES}
      DEPENDS initialise_directory_for_pybind11_modules ${MODULE_NAME}
      VERBATIM)
  endif()
endfunction()
