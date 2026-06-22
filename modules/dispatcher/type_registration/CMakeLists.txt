add_library(dispatcher_type_registration SHARED type_registration.cpp)
target_link_libraries(dispatcher_type_registration
                      PRIVATE oink_judge::dispatcher)
