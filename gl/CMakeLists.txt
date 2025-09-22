cmake_minimum_required(VERSION 3.15..4.0)

project(gl VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(../cmake/glfw.cmake)
link_glfw(${PROJECT_NAME} PUBLIC)

include(../cmake/glad.cmake)
link_glad(${PROJECT_NAME} PUBLIC)

include(../cmake/glm.cmake)
link_glm(${PROJECT_NAME} PUBLIC)

include(../cmake/spdlog.cmake)
LINK_SPDLOG_HEADER_ONLY(${PROJECT_NAME} PUBLIC)

# target_link_libraries(${PROJECT_NAME} PUBLIC text::text)

include(../cmake/imgui.cmake)
LINK_IMGUI(${PROJECT_NAME} PUBLIC)

add_subdirectory(src)
add_subdirectory("include")

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd5050 /wd4458>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token -fno-exceptions>
)

