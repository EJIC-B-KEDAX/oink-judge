add_library(oink_judge_boost_asio SHARED src/boost_asio.cpp)
add_library(oink_judge::boost_asio ALIAS oink_judge_boost_asio)

target_compile_definitions(
  oink_judge_boost_asio
  PUBLIC BOOST_ASIO_SEPARATE_COMPILATION)

target_link_libraries(oink_judge_boost_asio PUBLIC Boost::headers)
