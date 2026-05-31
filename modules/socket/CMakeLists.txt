cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::socket

set(SOCKET_PUBLIC_HEADERS
    include/oink_judge/socket/async_server.h
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
    include/oink_judge/socket/sessions/ssl_session.h)

set(SOCKET_SOURCES
    src/async_server.cpp
    src/boost_ssl_context.cpp
    src/byte_order.cpp
    src/client_config_utils.cpp
    src/connection_protocol.cpp
    src/connection_storage.cpp
    src/server_config_utils.cpp
    src/simple_connection_handler.cpp
    src/protocols/auth_required_protocol.cpp
    src/protocols/authorizing_protocol.cpp
    src/protocols/pinging_protocol.cpp
    src/protocols/ponging_protocol.cpp
    src/protocols/protocol_base.cpp
    src/protocols/protocol_decorator.cpp
    src/protocols/protocol_with_requests.cpp
    src/sessions/session_base.cpp
    src/sessions/ssl_session.cpp)

add_library(oink_judge_socket_headers INTERFACE ${SOCKET_PUBLIC_HEADERS})

target_include_directories(
  oink_judge_socket_headers
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(
  oink_judge_socket_headers
  INTERFACE oink_judge_logger_headers oink_judge_config_headers
            oink_judge_factory)

add_library(oink_judge_socket SHARED ${SOCKET_SOURCES})
add_library(oink_judge::socket ALIAS oink_judge_socket)

find_package(OpenSSL REQUIRED)

target_compile_features(oink_judge_socket PUBLIC cxx_std_20)
target_link_libraries(
  oink_judge_socket PUBLIC oink_judge_socket_headers oink_judge::logger
                           oink_judge::config OpenSSL::SSL OpenSSL::Crypto)

add_library(socket_type_registration SHARED
            type_registration/type_registration.cpp)
target_link_libraries(socket_type_registration PRIVATE oink_judge::socket)
target_compile_options(socket_type_registration PRIVATE -fPIC)

add_subdirectory(python_binding)
