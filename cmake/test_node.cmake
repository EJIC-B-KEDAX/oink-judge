set(TEST_NODE_HEADERS
    include/services/test_node/CompilationTest.h
    include/services/test_node/DefaultInvokerSessionEventHandler.h
    include/services/test_node/DefaultProblemBuilder.h
    include/services/test_node/DefaultTest.h
    include/services/test_node/DefaultVerdict.h
    include/services/test_node/DefaultVerdictBuilder.h
    include/services/test_node/enable_get_test_by_name_impl.h
    include/services/test_node/enable_get_test_by_name.hpp
    include/services/test_node/problem_utils.h
    include/services/test_node/ProblemBuilder.hpp
    include/services/test_node/ProblemTable.h
    include/services/test_node/ProblemTablesStorage.h
    include/services/test_node/SyncResultTest.h
    include/services/test_node/TableSubmissions.h
    include/services/test_node/Test.hpp
    include/services/test_node/Testset.h
    include/services/test_node/TestStorage.h
    include/services/test_node/verdict_utils.h
    include/services/test_node/Verdict.hpp
    include/services/test_node/VerdictBuilder.hpp
    include/services/test_node/VerdictType.hpp
)
set(TEST_NODE_SOURCES
    src/services/test_node/CompilationTest.cpp
    src/services/test_node/DefaultInvokerSessionEventHandler.cpp
    src/services/test_node/DefaultProblemBuilder.cpp
    src/services/test_node/DefaultTest.cpp
    src/services/test_node/DefaultVerdict.cpp
    src/services/test_node/DefaultVerdictBuilder.cpp
    src/services/test_node/enable_get_test_by_name_impl.cpp
    src/services/test_node/problem_utils.cpp
    src/services/test_node/ProblemTable.cpp
    src/services/test_node/ProblemTablesStorage.cpp
    src/services/test_node/SyncResultTest.cpp
    src/services/test_node/TableSubmissions.cpp
    src/services/test_node/Testset.cpp
    src/services/test_node/TestStorage.cpp
    src/services/test_node/verdict_utils.cpp
)

add_executable(test_node
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

        ${TEST_NODE_HEADERS}
        ${TEST_NODE_SOURCES}
        src/services/test_node/test_node_starter.cpp
)

target_compile_definitions(test_node PRIVATE CONFIG_DIR="../configs/test_node/config.json"
                                         CREDENTIALS_DIR="../configs/test_node/credentials.json")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBZIP REQUIRED libzip)
target_include_directories(test_node PRIVATE ${LIBZIP_INCLUDE_DIRS})
target_link_libraries(test_node PRIVATE ${LIBZIP_LIBRARIES})

target_include_directories(test_node PRIVATE /usr/local/include)
target_link_libraries(test_node PRIVATE pqxx pq)

find_package(PugiXML REQUIRED)
target_link_libraries(test_node PRIVATE pugixml)

find_package(OpenSSL REQUIRED)
target_link_libraries(test_node PRIVATE OpenSSL::SSL OpenSSL::Crypto)
