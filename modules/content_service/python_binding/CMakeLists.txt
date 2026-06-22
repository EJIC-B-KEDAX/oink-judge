add_oink_pybind_module(
  pybind11_content_service binding.cpp LINK_LIBRARIES
  oink_judge::content_service_client oink_judge::python_binding)
