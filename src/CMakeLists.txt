cmake_minimum_required(VERSION 3.15..4.0)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE logger::logger)

target_link_libraries(${PROJECT_NAME} PRIVATE gl::gl)


target_sources(${PROJECT_NAME} PRIVATE
 main.cpp
 logger.cpp
 input.cpp
 jfa.cpp
)

 target_precompile_headers(${PROJECT_NAME} PRIVATE
  <vector>
  <string>
  <array>
  <algorithm>
  <iostream>
  <gl/gl.hpp>
 )

add_subdirectory(shaders)
COPY_SHADERS(${PROJECT_NAME})
