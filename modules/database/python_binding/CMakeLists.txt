add_oink_pybind_module(
  pybind11_database binding.cpp LINK_LIBRARIES oink_judge::database
  oink_judge::python_binding::awaitable_support)
