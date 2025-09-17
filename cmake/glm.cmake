FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG master)
FetchContent_MakeAvailable(glm)

function(link_glm TARGET_NAME ACCESS)
  target_link_libraries(${TARGET_NAME} ${ACCESS} glm::glm)
endfunction()