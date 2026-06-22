# Module: oink_judge::plugin_manager

set(PLUGIN_MANAGER_PUBLIC_HEADERS
    include/oink_judge/plugin_manager/plugin_manager.h
    include/oink_judge/plugin_manager/config_utils.h)

set(PLUGIN_MANAGER_SOURCES src/plugin_manager.cpp src/config_utils.cpp)

# ------------------------------------------------------------------------------
# Plugin manager headers
# ------------------------------------------------------------------------------

add_library(oink_judge_plugin_manager_headers INTERFACE
            ${PLUGIN_MANAGER_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_plugin_manager_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(
  oink_judge_plugin_manager_headers INTERFACE oink_judge_logger_headers
                                              oink_judge_config_headers)

# ------------------------------------------------------------------------------
# Plugin manager
# ------------------------------------------------------------------------------

add_library(oink_judge_plugin_manager SHARED ${PLUGIN_MANAGER_SOURCES})
add_library(oink_judge::plugin_manager ALIAS oink_judge_plugin_manager)
target_link_libraries(
  oink_judge_plugin_manager PUBLIC oink_judge_plugin_manager_headers
                                   oink_judge::logger oink_judge::config)

add_subdirectory(python_binding)
