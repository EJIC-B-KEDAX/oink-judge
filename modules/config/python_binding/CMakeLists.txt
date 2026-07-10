add_oink_pybind_module(pybind11_config binding.cpp LINK_LIBRARIES
                       oink_judge::config)

target_link_libraries(pybind11_config PRIVATE oink_judge::python_binding)
