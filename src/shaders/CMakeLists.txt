include(shaders)

compile_shader(shaders GLSL ${PROJECT_NAME} SOURCES
  basic
  draw
  toUv
  jumpflood
  distance
  naive
  flatland_rc
  INCLUDES
  uv
  raymarching
)
