set(DATA_SENDER_HEADERS
    include/services/data_sender/ContentStorage.h
    include/services/data_sender/DataSenderSessionEventHandler.h
    include/services/data_sender/StorageSessionEventHandler.h
    include/services/data_sender/zip_utils.h
)
set(DATA_SENDER_SOURCES
    src/services/data_sender/ContentStorage.cpp
    src/services/data_sender/DataSenderSessionEventHandler.cpp
    src/services/data_sender/StorageSessionEventHandler.cpp
    src/services/data_sender/zip_utils.cpp
)

add_executable(data_sender_server
        ${FACTORY_HEADERS}
        ${FACTORY_SOURCES}
        ${CONFIG_HEADERS}
        ${CONFIG_SOURCES}
        ${SOCKET_HEADERS}
        ${SOCKET_SOURCES}
        ${DATABASE_HEADERS}
        ${DATABASE_SOURCES}

        ${DATA_SENDER_HEADERS}
        ${DATA_SENDER_SOURCES}
        src/services/simple_server_starter.cpp
)

target_compile_definitions(data_sender_server PRIVATE CONFIG_DIR="../configs/data_sender/config.json"
                                         CREDENTIALS_DIR="../configs/data_sender/credentials.json")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBZIP REQUIRED libzip)
target_include_directories(data_sender_server PRIVATE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(data_sender_server PRIVATE ${LIBZIP_LIBRARIES})

target_include_directories(data_sender_server PRIVATE /usr/local/include)
target_link_libraries(data_sender_server PRIVATE pqxx pq)

find_package(OpenSSL REQUIRED)
target_link_libraries(data_sender_server PRIVATE OpenSSL::SSL OpenSSL::Crypto)
