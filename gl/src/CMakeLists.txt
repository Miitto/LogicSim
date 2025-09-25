target_sources(${PROJECT_NAME}
  PRIVATE
  FILE_SET privateHeaders
  TYPE HEADERS

  PRIVATE
    buffer.cpp
    window.cpp
    gui.cpp
    logger.cpp
    vao.cpp
    shaders.cpp
)