# ------------------------------------------------------------------------------
# Content service tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_content_service_compare_manifests_tests
                test_compare_manifests.cpp oink_judge::content_service_common)
add_module_test(oink_judge_content_service_config_utils_tests
                test_config_utils.cpp oink_judge::content_service_common)
add_module_test(oink_judge_content_service_content_manifest_tests
                test_content_manifest.cpp oink_judge::content_service_common)
add_module_test(oink_judge_content_service_content_scanner_tests
                test_content_scanner.cpp oink_judge::content_service_common)
add_module_test(oink_judge_content_service_manifest_storage_tests
                test_manifest_storage.cpp oink_judge::content_service_common)

# ------------------------------------------------------------------------------
# Content service client tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_content_service_client_config_utils_tests
                test_client_config_utils.cpp oink_judge::content_service_client)

# ------------------------------------------------------------------------------
# Content service server tests
# ------------------------------------------------------------------------------

add_module_test(
  oink_judge_content_service_server_tests test_content_service_server.cpp
  oink_judge::content_service_server)
target_link_libraries(oink_judge_content_service_server_tests
                      PRIVATE oink_judge::content_service_client)

# ------------------------------------------------------------------------------
# Content service polygon converter tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_content_service_polygon_converter_tests
                test_polygon_converter.cpp oink_judge::factory)
target_sources(
  oink_judge_content_service_polygon_converter_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/problem_package_converter/polygon_converter.cpp
)
target_include_directories(oink_judge_content_service_polygon_converter_tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(oink_judge_content_service_polygon_converter_tests
                      PRIVATE pugixml)

# ------------------------------------------------------------------------------
# Content service test server
# ------------------------------------------------------------------------------

add_executable(content_service_test_server content_service_test_server.cpp)
target_link_libraries(content_service_test_server
                      PRIVATE oink_judge::content_service_server)

# ------------------------------------------------------------------------------
# Content service client integration tests
# ------------------------------------------------------------------------------

add_executable(oink_judge_content_service_client_integration_tests
               test_content_service_client_integration.cpp)
target_link_libraries(
  oink_judge_content_service_client_integration_tests
  PRIVATE oink_judge::content_service_client GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(
  oink_judge_content_service_client_integration_tests
  PROPERTIES
    ENVIRONMENT
    CONTENT_SERVICE_TEST_SERVER=$<TARGET_FILE:content_service_test_server>)

# ------------------------------------------------------------------------------
# Content service python integration tests
# ------------------------------------------------------------------------------

add_test(
  NAME oink_judge_content_service_python_modules_prepare
  COMMAND
    ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
    content_service_test_server install_pybind11_content_service
    install_pybind11_config install_pybind11_logger install_pybind11_factory
    install_pybind11_utils install_pybind11_utils_grpc)
set_tests_properties(oink_judge_content_service_python_modules_prepare
                     PROPERTIES FIXTURES_SETUP oink_judge_content_service_python_modules)

add_test(
  NAME oink_judge_content_service_python_integration_tests
  COMMAND
    ${CMAKE_COMMAND} -E env
    "PYTHONPATH=${PATH_TO_INSTALL_PYBIND11_MODULES}:${CMAKE_SOURCE_DIR}"
    "CONTENT_SERVICE_TEST_SERVER=$<TARGET_FILE:content_service_test_server>"
    ${Python3_EXECUTABLE} -m pytest
    ${CMAKE_CURRENT_SOURCE_DIR}/test_content_service_python_integration.py -v)
set_tests_properties(
  oink_judge_content_service_python_integration_tests
  PROPERTIES FIXTURES_REQUIRED oink_judge_content_service_python_modules)

# ------------------------------------------------------------------------------
# Copy resources
# ------------------------------------------------------------------------------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
