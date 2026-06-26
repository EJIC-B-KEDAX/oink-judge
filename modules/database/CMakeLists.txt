# Module: oink_judge::database

set(DATABASE_PUBLIC_HEADERS
    include/oink_judge/database/config_utils.h
    include/oink_judge/database/query_result.h
    include/oink_judge/database/query_result.inl
    include/oink_judge/database/query_param.h
    include/oink_judge/database/query_param.inl
    include/oink_judge/database/statement_action_log.h
    include/oink_judge/database/libpq_connection.h
    include/oink_judge/database/pooled_connection.h
    include/oink_judge/database/connection_pool.h
    include/oink_judge/database/query.h
    include/oink_judge/database/query.inl
    include/oink_judge/database/table_submissions.h)

set(DATABASE_SOURCES
    src/config_utils.cpp
    src/query_result.cpp
    src/query_param.cpp
    src/statement_action_log.cpp
    src/libpq_connection.cpp
    src/pooled_connection.cpp
    src/connection_pool.cpp
    src/query.cpp
    src/table_submissions.cpp)

# ------------------------------------------------------------------------------
# Database headers
# ------------------------------------------------------------------------------

add_library(oink_judge_database_headers INTERFACE ${DATABASE_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_database_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(
  oink_judge_database_headers
  INTERFACE oink_judge_logger_headers oink_judge_config_headers
            oink_judge::boost_asio)

# ------------------------------------------------------------------------------
# Database
# ------------------------------------------------------------------------------

add_library(oink_judge_database SHARED ${DATABASE_SOURCES})
add_library(oink_judge::database ALIAS oink_judge_database)

target_link_libraries(
  oink_judge_database
  PUBLIC oink_judge_database_headers PkgConfig::LIBPQXX pq oink_judge::logger
         oink_judge::config oink_judge::boost_asio)

add_subdirectory(python_binding)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
