cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::socket
# This CMakeLists provides a reusable `oink_judge::socket` target with public
# headers under `include/oink/socket` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
set(SOCKET_PUBLIC_HEADERS
	include/oink_judge/socket/async_server.h
    include/oink_judge/socket/boost_io_context.h
    include/oink_judge/socket/boost_ssl_context.h
    include/oink_judge/socket/byte_order.h
    include/oink_judge/socket/connection_handler.hpp
    include/oink_judge/socket/connection_protocol.h
    include/oink_judge/socket/connection_storage.h
    include/oink_judge/socket/protocol.hpp
    include/oink_judge/socket/protocol.inl
    include/oink_judge/socket/session.hpp
    include/oink_judge/socket/session.inl
    include/oink_judge/socket/simple_connection_handler.h
    include/oink_judge/socket/protocols/auth_required_protocol.h
    include/oink_judge/socket/protocols/authorizing_protocol.h
    include/oink_judge/socket/protocols/pinging_protocol.h
    include/oink_judge/socket/protocols/ponging_protocol.h
    include/oink_judge/socket/protocols/protocol_base.h
    include/oink_judge/socket/protocols/protocol_decorator.h
    include/oink_judge/socket/protocols/protocol_with_requests.h
    include/oink_judge/socket/sessions/session_base.h
    include/oink_judge/socket/sessions/ssl_session.h
)

set(SOCKET_SOURCES
	src/async_server.cpp
    src/boost_io_context.cpp
    src/boost_ssl_context.cpp
    src/byte_order.cpp
    src/connection_protocol.cpp
    src/connection_storage.cpp
    src/simple_connection_handler.cpp
    src/protocols/auth_required_protocol.cpp
    src/protocols/authorizing_protocol.cpp
    src/protocols/pinging_protocol.cpp
    src/protocols/ponging_protocol.cpp
    src/protocols/protocol_base.cpp
    src/protocols/protocol_decorator.cpp
    src/protocols/protocol_with_requests.cpp
    src/sessions/session_base.cpp
    src/sessions/ssl_session.cpp
)

add_library(oink_judge_socket STATIC ${SOCKET_SOURCES} ${SOCKET_PUBLIC_HEADERS})
add_library(oink_judge::socket ALIAS oink_judge_socket)

target_include_directories(oink_judge_socket
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_socket PUBLIC cxx_std_20)

target_link_libraries(oink_judge_socket PUBLIC oink_judge::logger oink_judge::config oink_judge::factory)

option(OINK_SOCKET_INSTALL "Install oink_judge::socket target" ON)
if(OINK_SOCKET_INSTALL)
	install(TARGETS oink_judge_socket
        EXPORT oinkJudgeTargets
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include
	)

	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
		DESTINATION include
		FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
	)

	install(EXPORT oinkJudgeTargets
		NAMESPACE oink_judge::
		FILE oinkJudgeTargets-socket.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()
