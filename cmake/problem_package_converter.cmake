set(PROBLEM_PACKAGE_CONVERTER_HEADERS
    include/services/data_sender/problem_package_converter/PackageConverter.hpp
    include/services/data_sender/problem_package_converter/PolygonConverter.h
)
set(PROBLEM_PACKAGE_CONVERTER_SOURCES
    src/services/data_sender/problem_package_converter/PolygonConverter.cpp
)

add_executable(problem_package_converter
        ${FACTORY_HEADERS}
        ${FACTORY_SOURCES}

        ${PROBLEM_PACKAGE_CONVERTER_HEADERS}
        ${PROBLEM_PACKAGE_CONVERTER_SOURCES}
        src/services/data_sender/problem_package_converter/problem_package_converter.cpp
)

find_package(PkgConfig REQUIRED)

find_package(PugiXML REQUIRED)
target_link_libraries(problem_package_converter PRIVATE pugixml)
