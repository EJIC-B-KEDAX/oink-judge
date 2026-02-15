cmake_minimum_required(VERSION 3.16)

# Module: oink_judge::factory

set(FACTORY_PUBLIC_HEADERS
	include/oink_judge/factory/parameterized_type_factory.hpp
    include/oink_judge/factory/parameterized_type_factory.inl
	include/oink_judge/factory/type_factory.hpp
    include/oink_judge/factory/type_factory.inl
)

add_library(oink_judge_factory INTERFACE ${FACTORY_PUBLIC_HEADERS})
add_library(oink_judge::factory ALIAS oink_judge_factory)

target_include_directories(oink_judge_factory INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(oink_judge_factory INTERFACE cxx_std_20)
