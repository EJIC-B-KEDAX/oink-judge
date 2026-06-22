add_library(content_service_server_type_registration SHARED
            server_type_registration.cpp)
target_link_libraries(content_service_server_type_registration
                      PRIVATE oink_judge::content_service_server)

add_library(content_service_client_type_registration SHARED
            client_type_registration.cpp)
target_link_libraries(content_service_client_type_registration
                      PRIVATE oink_judge::content_service_client)
