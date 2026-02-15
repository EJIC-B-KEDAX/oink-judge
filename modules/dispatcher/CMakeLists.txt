cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::dispatcher

set(DISPATCHER_PUBLIC_HEADERS
    include/oink_judge/dispatcher/invoker.h
    include/oink_judge/dispatcher/problem_submission_manager.hpp
    include/oink_judge/dispatcher/protocol_with_fastapi.h
    include/oink_judge/dispatcher/protocol_with_invoker.h
	include/oink_judge/dispatcher/send_submission_to_invoker.h
    include/oink_judge/dispatcher/testing_queue.h
)

set(DISPATCHER_SOURCES
    src/invoker.cpp
    src/protocol_with_fastapi.cpp
    src/protocol_with_invoker.cpp
    src/send_submission_to_invoker.cpp
	src/server_starter.cpp
    src/testing_queue.cpp
)

add_executable(oink_judge_dispatcher ${DISPATCHER_SOURCES} ${DISPATCHER_PUBLIC_HEADERS})

target_include_directories(oink_judge_dispatcher PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(oink_judge_dispatcher PUBLIC cxx_std_20)
target_link_libraries(oink_judge_dispatcher PUBLIC oink_judge::logger oink_judge::config oink_judge::factory oink_judge::socket oink_judge::content_service_client oink_judge::database)
