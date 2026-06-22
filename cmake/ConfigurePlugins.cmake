function(configure_service_plugins TARGET_NAME SERVICE_NAME)
  cmake_parse_arguments(ARG "" "" "TYPE_REGISTRATIONS" ${ARGN})

  set(PLUGIN_DIR ${OINK_JUDGE_PLUGINS_DIR}/${SERVICE_NAME})
  set(PLUGIN_COMMANDS COMMAND ${CMAKE_COMMAND} -E make_directory
                              ${PLUGIN_DIR})

  foreach(REGISTRATION_TARGET ${ARG_TYPE_REGISTRATIONS})
    set_target_properties(
      ${REGISTRATION_TARGET}
      PROPERTIES BUILD_RPATH "$ORIGIN/../../lib"
                 INSTALL_RPATH "$ORIGIN/../../lib")
    list(APPEND PLUGIN_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy
         $<TARGET_FILE:${REGISTRATION_TARGET}> ${PLUGIN_DIR}/)
  endforeach()

  add_custom_target(${TARGET_NAME} ALL ${PLUGIN_COMMANDS}
                    DEPENDS ${ARG_TYPE_REGISTRATIONS}
                    VERBATIM)
endfunction()
