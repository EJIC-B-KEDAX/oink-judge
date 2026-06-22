# Module: oink_judge::utils

set(UTILS_PUBLIC_HEADERS
    include/oink_judge/utils/async.h include/oink_judge/utils/crypto.h
    include/oink_judge/utils/filesystem.h)

set(UTILS_SOURCES src/async.cpp src/crypto.cpp src/filesystem.cpp)

# ------------------------------------------------------------------------------
# Utils headers
# ------------------------------------------------------------------------------

add_library(oink_judge_utils_headers INTERFACE ${UTILS_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_utils_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(oink_judge_utils_headers
                      INTERFACE oink_judge_logger_headers oink_judge::boost_asio)

# ------------------------------------------------------------------------------
# Utils
# ------------------------------------------------------------------------------

add_library(oink_judge_utils SHARED ${UTILS_SOURCES})
add_library(oink_judge::utils ALIAS oink_judge_utils)

target_link_libraries(
  oink_judge_utils
  PUBLIC oink_judge_utils_headers libzip::zip PkgConfig::SODIUM
         oink_judge::logger oink_judge::boost_asio Boost::filesystem
         Boost::process)

add_subdirectory(python_binding)
add_subdirectory(grpc)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
