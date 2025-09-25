cmake_minimum_required(VERSION 3.15..4.0)

# Output compile commands
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

find_package(Git)
if(NOT Git_FOUND)
  message(STATUS "Git not found, adding cmake/FindGit.cmake to module path to use VS2022 Git")
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VSGit")
  find_package(Git)
endif()

find_program(SCCACHE_FOUND sccache)

if(SCCACHE_FOUND)
  message(STATUS "Using sccache for compilation")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
  set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})

  # For MSVC debug information format
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
  cmake_policy(SET CMP0141 NEW)
else()
  message(STATUS "sccache not found, using default compiler")
endif()

project(LogicSim VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(logger)
add_subdirectory(gl)
add_subdirectory(src)

include(enableWarnings)
ENABLE_WARNINGS(${PROJECT_NAME})

add_compile_definitions(CMAKE_PROJECT_DIR=${CMAKE_PROJECT_DIR})
