function(link_glad TARGET_NAME)
  add_subdirectory(../vendor/glad glad_cmake)

  target_link_libraries(${TARGET_NAME} PRIVATE glad)
endfunction()