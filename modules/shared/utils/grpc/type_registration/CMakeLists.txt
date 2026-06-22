add_library(utils_grpc_type_registration SHARED type_registration.cpp)
target_link_libraries(utils_grpc_type_registration
                      PRIVATE oink_judge::utils::grpc)
