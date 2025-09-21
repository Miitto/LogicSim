add_subdirectory(../vendor/imgui imgui_cmake)

function(link_imgui TARGET_NAME ACCESS)
  target_link_libraries(${TARGET_NAME} ${ACCESS} imgui::imgui)
endfunction()