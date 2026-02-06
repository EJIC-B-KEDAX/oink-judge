cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::auth_service

set(AUTH_SERVICE_PUBLIC_HEADERS
    include/oink_judge/auth_service/auth_manager.h
    include/oink_judge/auth_service/handle_request.h
    include/oink_judge/auth_service/protocol_with_fastapi.h
    include/oink_judge/auth_service/session.h
	include/oink_judge/auth_service/table_sessions.h
    include/oink_judge/auth_service/table_users.h
)

set(AUTH_SERVICE_SOURCES
    src/auth_manager.cpp
    src/handle_request.cpp
    src/protocol_with_fastapi.cpp
    src/server_starter.cpp
    src/session.cpp
    src/table_sessions.cpp
    src/table_users.cpp
)

add_executable(oink_judge_auth_service ${AUTH_SERVICE_SOURCES} ${AUTH_SERVICE_PUBLIC_HEADERS})

target_include_directories(oink_judge_auth_service PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(oink_judge_auth_service PUBLIC cxx_std_20)
target_link_libraries(oink_judge_auth_service PUBLIC oink_judge::logger oink_judge::config oink_judge::utils oink_judge::socket oink_judge::database)
