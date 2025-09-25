cmake_minimum_required(VERSION 3.15..4.0)

add_executable(LogicSim)

target_link_libraries(LogicSim PRIVATE logger::logger)

target_link_libraries(LogicSim PRIVATE gl::gl)


target_sources(LogicSim PRIVATE
 main.cpp
 logger.cpp
 input.cpp
)

 target_precompile_headers(LogicSim PRIVATE
  <vector>
  <string>
  <array>
  <algorithm>
  <iostream>
  <gl/gl.hpp>
 )

add_subdirectory(shaders)
COPY_SHADERS(LogicSim)
