cmake_minimum_required(VERSION 3.15..4.0)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE gl::gl)

get_target_property(hdrs gl HEADER_SET)
message(STATUS "Interface headers: ${hdrs}")

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)