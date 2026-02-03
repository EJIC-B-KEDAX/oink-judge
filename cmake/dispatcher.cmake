set(DISPATCHER_HEADERS
    include/services/dispatcher/BasicProblemSubmissionManager.h
    include/services/dispatcher/Invoker.h
    include/services/dispatcher/ProblemSubmissionManager.hpp
    include/services/dispatcher/ProtocolWithFastAPI.h
    include/services/dispatcher/ProtocolWithInvoker.h
    include/services/dispatcher/TestingQueue.h
)
set(DISPATCHER_SOURCES
    src/services/dispatcher/BasicProblemSubmissionManager.cpp
    src/services/dispatcher/Invoker.cpp
    src/services/dispatcher/ProtocolWithFastAPI.cpp
    src/services/dispatcher/ProtocolWithInvoker.cpp
    src/services/dispatcher/TestingQueue.cpp
)

add_executable(dispatcher_server
        ${FACTORY_HEADERS}
        ${FACTORY_SOURCES}
        ${SOCKET_HEADERS}
        ${SOCKET_SOURCES}
        ${DATABASE_HEADERS}
        ${DATABASE_SOURCES}
        ${DATA_SENDER_HEADERS}
        ${DATA_SENDER_SOURCES}
        ${UTILS_HEADERS}
        ${UTILS_SOURCES}

        ${DISPATCHER_HEADERS}
        ${DISPATCHER_SOURCES}
        src/services/simple_server_starter.cpp
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBZIP REQUIRED libzip)
target_include_directories(dispatcher_server PRIVATE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(dispatcher_server PRIVATE ${LIBZIP_LIBRARIES})

pkg_check_modules(SODIUM REQUIRED libsodium)
target_include_directories(dispatcher_server PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(dispatcher_server PRIVATE ${SODIUM_LIBRARIES})

target_include_directories(dispatcher_server PRIVATE /usr/local/include)
target_link_libraries(dispatcher_server PRIVATE pqxx pq)

find_package(PugiXML REQUIRED)
target_link_libraries(dispatcher_server PRIVATE pugixml)

find_package(OpenSSL REQUIRED)
target_link_libraries(dispatcher_server PRIVATE OpenSSL::SSL OpenSSL::Crypto)
