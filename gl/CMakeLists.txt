cmake_minimum_required(VERSION 3.15..4.0)

project(gl VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(../cmake/glfw.cmake)
link_glfw(${PROJECT_NAME})

add_subdirectory(src)
add_subdirectory("include")

