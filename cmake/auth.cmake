set(AUTH_HEADERS
    include/services/auth/TableUsers.h
    include/services/auth/TableSessions.h
    include/services/auth/AuthManager.h
    include/services/auth/Session.h
    include/services/auth/SessionWithFastAPIEventHandler.h
    include/services/auth/HandleRequest.h
)
set(AUTH_SOURCES
    src/services/auth/TableUsers.cpp
    src/services/auth/TableSessions.cpp
    src/services/auth/AuthManager.cpp
    src/services/auth/Session.cpp
    src/services/auth/SessionWithFastAPIEventHandler.cpp
    src/services/auth/HandleRequest.cpp
)

add_executable(auth_server
        ${FACTORY_HEADERS}
        ${FACTORY_SOURCES}
        ${CONFIG_HEADERS}
        ${CONFIG_SOURCES}
        ${SOCKET_HEADERS}
        ${SOCKET_SOURCES}
        ${DATABASE_HEADERS}
        ${DATABASE_SOURCES}

        ${AUTH_HEADERS}
        ${AUTH_SOURCES}
        src/services/simple_server_starter.cpp
)

target_compile_definitions(auth_server PRIVATE CONFIG_DIR="../configs/auth/config.json"
                                         CREDENTIALS_DIR="../configs/auth/credentials.json")

find_package(PkgConfig REQUIRED)

pkg_check_modules(SODIUM REQUIRED libsodium)
target_include_directories(auth_server PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(auth_server PRIVATE ${SODIUM_LIBRARIES})

target_include_directories(auth_server PRIVATE /usr/local/include)
target_link_libraries(auth_server PRIVATE pqxx pq)

find_package(OpenSSL REQUIRED)
target_link_libraries(auth_server PRIVATE OpenSSL::SSL OpenSSL::Crypto)
