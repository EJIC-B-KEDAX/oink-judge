cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::config

set(DATABASE_PUBLIC_HEADERS
    include/oink_judge/database/config_utils.h
    include/oink_judge/database/database.h
    include/oink_judge/database/database.inl
    include/oink_judge/database/table_submissions.h)

set(DATABASE_SOURCES src/config_utils.cpp src/database.cpp
                     src/table_submissions.cpp)

add_library(oink_judge_database_headers INTERFACE ${DATABASE_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_database_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(
  oink_judge_database_headers INTERFACE oink_judge_logger_headers
                                        oink_judge_config_headers)

add_library(oink_judge_database SHARED ${DATABASE_SOURCES})
add_library(oink_judge::database ALIAS oink_judge_database)

target_compile_features(oink_judge_database PUBLIC cxx_std_20)

find_package(PugiXML REQUIRED)
target_link_libraries(
  oink_judge_database PUBLIC oink_judge_database_headers pqxx pq pugixml
                             oink_judge::logger oink_judge::config)
