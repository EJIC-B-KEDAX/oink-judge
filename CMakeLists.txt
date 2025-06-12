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
        include/backend/auth/HandleRequest.h
        include/socket/ClientSocket.h
        include/socket/LocalSocket.h
        include/config/Config.h
)

set(SOURCES
        src/backend/auth/auth.cpp
        src/backend/auth/AuthDB.cpp
        src/backend/auth/AuthManager.cpp
        src/backend/auth/Session.cpp
        src/backend/auth/HandleRequest.cpp
        src/socket/ClientSocket.cpp
        src/socket/LocalSocket.cpp
        src/config/Config.cpp
)

add_executable(auth_server
        ${HEADERS}
        ${SOURCES}
)

target_include_directories(auth_server PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(auth_server PRIVATE sodium SQLite::SQLite3 ${SODIUM_LIBRARIES})