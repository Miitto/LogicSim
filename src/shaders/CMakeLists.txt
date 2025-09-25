include(shaders)

compile_shader(shaders GLSL ${PROJECT_NAME} SOURCES
  basic
  draw
)
