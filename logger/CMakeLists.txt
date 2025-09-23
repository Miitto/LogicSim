cmake_minimum_required(VERSION 3.15..4.0)

project(logger VERSION 0.1.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)

add_library(logger::logger ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include FILES include/logger/log.hpp)

include(spdlog)
LINK_SPDLOG(${PROJECT_NAME} INTERFACE)
