cmake_minimum_required(VERSION 3.15..4.0)

project(renderer)

add_library(${PROJECT_NAME})
add_library(renderer::renderer ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC gl::gl)

include(enableWarnings)
ENABLE_WARNINGS(${PROJECT_NAME})

add_subdirectory(src)
add_subdirectory(include)