add_subdirectory(../vendor/glad glad_cmake)

function(link_glad TARGET_NAME ACCESS)
  target_link_libraries(${TARGET_NAME} ${ACCESS} glad::glad)
endfunction()