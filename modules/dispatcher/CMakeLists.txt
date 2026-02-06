cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::dispatcher
# This CMakeLists provides a reusable `oink_judge::dispatcher` target with public
# headers under `include/oink/dispatcher` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
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
    src/testing_queue.cpp
)

add_library(oink_judge_dispatcher STATIC ${DISPATCHER_SOURCES} ${DISPATCHER_PUBLIC_HEADERS})
add_library(oink_judge::dispatcher ALIAS oink_judge_dispatcher)

target_include_directories(oink_judge_dispatcher
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_dispatcher PUBLIC cxx_std_20)

target_link_libraries(oink_judge_dispatcher PUBLIC oink_judge::logger oink_judge::config oink_judge::factory oink_judge::socket oink_judge::content_service oink_judge::database)

option(OINK_DISPATCHER_INSTALL "Install oink_judge::dispatcher target" ON)
if(OINK_DISPATCHER_INSTALL)
	install(TARGETS oink_judge_dispatcher
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
		FILE oinkJudgeTargets-dispatcher.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()
