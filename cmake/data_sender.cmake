set(DATA_SENDER_HEADERS
    include/services/data_sender/ContentManifest.h
    include/services/data_sender/ContentStorage.h
    include/services/data_sender/DataSenderProtocol.h
    include/services/data_sender/ManifestStorage.h
    include/services/data_sender/StorageProtocol.h
)
set(DATA_SENDER_SOURCES
    src/services/data_sender/ContentManifest.cpp
    src/services/data_sender/ContentStorage.cpp
    src/services/data_sender/DataSenderProtocol.cpp
    src/services/data_sender/ManifestStorage.cpp
    src/services/data_sender/StorageProtocol.cpp
)

add_executable(data_sender_server
        ${FACTORY_HEADERS}
        ${FACTORY_SOURCES}
        ${SOCKET_HEADERS}
        ${SOCKET_SOURCES}
        ${DATABASE_HEADERS}
        ${DATABASE_SOURCES}
        ${UTILS_HEADERS}
        ${UTILS_SOURCES}

        ${DATA_SENDER_HEADERS}
        ${DATA_SENDER_SOURCES}
        src/services/simple_server_starter.cpp
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBZIP REQUIRED libzip)
target_include_directories(data_sender_server PRIVATE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(data_sender_server PRIVATE ${LIBZIP_LIBRARIES})

pkg_check_modules(SODIUM REQUIRED libsodium)
target_include_directories(data_sender_server PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(data_sender_server PRIVATE ${SODIUM_LIBRARIES})

target_include_directories(data_sender_server PRIVATE /usr/local/include)
target_link_libraries(data_sender_server PRIVATE pqxx pq)

find_package(PugiXML REQUIRED)
target_link_libraries(data_sender_server PRIVATE pugixml)

find_package(OpenSSL REQUIRED)
target_link_libraries(data_sender_server PRIVATE OpenSSL::SSL OpenSSL::Crypto)
