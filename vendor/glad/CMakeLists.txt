cmake_minimum_required(VERSION 3.15..4.0)

project(glad VERSION 0.1.0 LANGUAGES C)

add_library(glad)
add_library(glad::glad ALIAS glad)

target_sources(glad
  PRIVATE
    src/glad.c
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS
  include
)
target_precompile_headers(glad
  PUBLIC
    <glad/glad.h>
)