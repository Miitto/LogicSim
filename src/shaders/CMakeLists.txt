include(shaders)

compile_shader(shaders GLSL ${PROJECT_NAME} SOURCES
  basic
  draw
  naive
  toUv
  jumpflood
  INCLUDES
  uv
)
