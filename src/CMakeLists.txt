cmake_minimum_required(VERSION 3.15..4.0)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE logger::logger)

target_link_libraries(${PROJECT_NAME} PRIVATE renderer::renderer)


target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
 "logger.cpp")

 target_precompile_headers(${PROJECT_NAME} PRIVATE
  <vector>
  <string>
  <array>
  <algorithm>
  <iostream>
 )