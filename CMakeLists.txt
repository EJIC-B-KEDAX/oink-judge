cmake_minimum_required(VERSION 3.28)
project(oink-judge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(SODIUM REQUIRED libsodium)


set(HEADERS
        include/backend/auth/TableUsers.h
        include/backend/auth/AuthManager.h
        include/backend/auth/Session.h
        include/backend/auth/HandleRequest.h
        include/socket/BoostIOContext.h
        include/socket/BoostSSLContext.h
        include/config/Config.h
        include/database/DataBase.h
        include/database/DataBase.inl
        include/backend/management/ProblemTable.h
        include/backend/management/ProblemConfig.h
        include/backend/testing/TestsetVerdict.h
        include/backend/testing/Test.h
        include/backend/testing/Verdict.h
        include/backend/testing/Subtask.h
        include/backend/testing/Testset.h
        include/backend/testing/OutputTest.h
        include/backend/testing/SubtaskVerdict.h
        include/backend/testing/SimpleSubtask.h
        include/backend/testing/SimpleTestset.h
        include/backend/management/ICPCProblem.h
        include/backend/management/Submission.h
        include/backend/auth/TableSessions.h
        include/backend/management/TestingQueue.h
        include/socket/AsyncServer.h
        include/TypeFactory.h
        include/TypeFactory.inl
        include/ParameterizedTypeFactory.h
        include/ParameterizedTypeFactory.inl
        include/socket/Session.h
        include/socket/ConnectionHandler.hpp
        include/socket/SessionEventHandler.hpp
        include/socket/sessions/BasicSession.h
        include/backend/management/Invoker.h
        include/socket/sessions/SessionBase.h
        include/socket/ConnectionStorage.h
        include/backend/management/DefaultSessionWithInvokerEventHandler.h
        include/backend/management/ManagementConnectionHandler.h
)

set(SOURCES
        src/backend/auth/TableUsers.cpp
        src/backend/auth/AuthManager.cpp
        src/backend/auth/Session.cpp
        src/backend/auth/HandleRequest.cpp
        src/socket/BoostIOContext.cpp
        src/socket/BoostSSLContext.cpp
        src/config/Config.cpp
        src/backend/management/Problem.cpp
        src/database/DataBase.cpp
        src/backend/management/ProblemTable.cpp
        src/backend/testing/Test.cpp
        src/backend/testing/OutputTest.cpp
        src/backend/management/ProblemConfig.cpp
        src/backend/testing/SimpleSubtask.cpp
        src/backend/testing/SimpleTestset.cpp
        src/backend/testing/Subtask.cpp
        src/backend/testing/SubtaskVerdict.cpp
        src/backend/testing/Testset.cpp
        src/backend/testing/TestsetVerdict.cpp
        src/backend/testing/Verdict.cpp
        src/backend/management/ICPCProblem.cpp
        src/backend/management/Submission.cpp
        src/backend/auth/TableSessions.cpp
        src/backend/management/TestingQueue.cpp
        src/socket/AsyncServer.cpp
        src/socket/Session.cpp
        src/socket/sessions/BasicSession.cpp
        src/backend/management/Invoker.cpp
        src/socket/sessions/SessionBase.cpp
        src/socket/ConnectionStorage.cpp
        src/backend/management/DefaultSessionWithInvokerEventHandler.cpp
)

# add_executable(auth_server
#         ${HEADERS}
#         ${SOURCES}
#         src/backend/auth/auth.cpp
# )

# add_executable(management_server
#         ${HEADERS}
#         ${SOURCES}
#         src/backend/management/management_test.cpp
# )

# target_include_directories(auth_server PRIVATE ${SODIUM_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
# target_link_libraries(auth_server PRIVATE sodium ${SODIUM_LIBRARIES} pugixml pqxx pq OpenSSL::SSL OpenSSL::Crypto)


# target_include_directories(management_server PRIVATE ${SODIUM_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
# target_link_libraries(management_server PRIVATE sodium ${SODIUM_LIBRARIES} pugixml pqxx pq OpenSSL::SSL OpenSSL::Crypto)