# Module: oink_judge::config

set(CONFIG_PUBLIC_HEADERS
    include/oink_judge/config/common_utils.h
    include/oink_judge/config/common_utils.inl
    include/oink_judge/config/config.h
    include/oink_judge/config/problem_config_utils.h
    include/oink_judge/config/logger_utils.h
    include/oink_judge/config/server_utils.h)

set(CONFIG_SOURCES
    src/common_utils.cpp src/config.cpp src/problem_config_utils.cpp
    src/logger_utils.cpp src/server_utils.cpp)

# ------------------------------------------------------------------------------
# Config headers
# ------------------------------------------------------------------------------

add_library(oink_judge_config_headers INTERFACE ${CONFIG_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_config_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(oink_judge_config_headers
                      INTERFACE oink_judge_logger_headers)

# ------------------------------------------------------------------------------
# Config
# ------------------------------------------------------------------------------

add_library(oink_judge_config SHARED ${CONFIG_SOURCES})
add_library(oink_judge::config ALIAS oink_judge_config)

target_link_libraries(
  oink_judge_config PUBLIC oink_judge_config_headers pugixml oink_judge::logger
                           oink_judge::utils)

add_subdirectory(python_binding)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
