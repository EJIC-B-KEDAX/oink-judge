cmake_minimum_required(VERSION 3.8)
project(factory_headers LANGUAGES CXX)

set(FACTORY_PUBLIC_HEADERS
	include/oink_judge/factory/parameterized_type_factory.hpp
    include/oink_judge/factory/parameterized_type_factory.inl
	include/oink_judge/factory/type_factory.hpp
    include/oink_judge/factory/type_factory.inl
)

# Header-only interface library for the "factory" module
add_library(oink_judge_factory INTERFACE ${FACTORY_PUBLIC_HEADERS})
add_library(oink_judge::factory ALIAS oink_judge_factory)

target_include_directories(oink_judge_factory INTERFACE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_factory INTERFACE cxx_std_20)

option(OINK_FACTORY_INSTALL "Install oink_judge::factory target" ON)
if(OINK_FACTORY_INSTALL)
	install(TARGETS oink_judge_factory
		EXPORT oinkJudgeTargets
		INCLUDES DESTINATION include
	)

	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
		DESTINATION include
		FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
	)

	install(EXPORT oinkJudgeTargets
		NAMESPACE oink_judge::
		FILE oinkJudgeTargets-factory.cmake
		DESTINATION lib/cmake/oink_judge
	)
endif()
