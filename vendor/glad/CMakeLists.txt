cmake_minimum_required(VERSION 3.15..4.0)

project(glad VERSION 0.1.0 LANGUAGES C)

add_library(glad)

target_sources(glad
  PRIVATE
    src/glad.c
  PUBLIC
  FILE_SET HEADERS
  include/glad/glad.h
  include/KHR/khrplatform.h
)