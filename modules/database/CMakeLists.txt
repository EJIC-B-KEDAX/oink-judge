cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::config
# This CMakeLists provides a reusable `oink_judge::config` target with public
# headers under `include/oink/config` and sources under `src/`.

# Source / header lists - keep these minimal; add files as needed
set(DATABASE_PUBLIC_HEADERS
	include/oink_judge/database/database.h
    include/oink_judge/database/database.inl
	include/oink_judge/database/table_submissions.h
)

set(DATABASE_SOURCES
	src/database.cpp
    src/table_submissions.cpp
)

add_library(oink_judge_database STATIC ${DATABASE_SOURCES} ${DATABASE_PUBLIC_HEADERS})
add_library(oink_judge::database ALIAS oink_judge_database)

target_include_directories(oink_judge_database
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_database PUBLIC cxx_std_20)

find_package(PugiXML REQUIRED)
target_link_libraries(oink_judge_database PUBLIC pugixml oink_judge::logger oink_judge::config)

option(OINK_DATABASE_INSTALL "Install oink_judge::database target" ON)
if(OINK_DATABASE_INSTALL)
	install(TARGETS oink_judge_database
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
		FILE oinkJudgeTargets-database.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()
