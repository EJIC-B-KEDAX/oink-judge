cmake_minimum_required(VERSION 3.28)
project(oink-judge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)


set(HEADERS
        include/backend/auth/AuthDB.h
        include/backend/auth/AuthManager.h
        include/backend/auth/Session.h
)

set(SOURCES
        src/backend/auth/AuthDB.cpp
        src/backend/auth/AuthManager.cpp
        src/backend/auth/Session.cpp
)

add_executable(oink-judge
        oink-judge.cpp
        ${HEADERS}
        ${SOURCES}
)

target_include_directories(oink-judge PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(oink-judge PRIVATE sodium SQLite::SQLite3 ${SODIUM_LIBRARIES})