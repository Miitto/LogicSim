target_sources(${PROJECT_NAME}
  PRIVATE
  FILE_SET privateHeaders
  TYPE HEADERS

  PRIVATE
    window.cpp
    gui.cpp
)