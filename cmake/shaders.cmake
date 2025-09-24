find_program(SLANGC_EXECUTABLE NAMES slangc)

if(NOT SLANGC_EXECUTABLE})
  message(STATUS "slangc not found in PATH, looking in ${CMAKE_SOURCE_DIR}/external/slang/bin")
  find_program(SLANGC_EXECUTABLE NAMES slangc PATHS ${CMAKE_SOURCE_DIR}/external/slang/bin REQUIRED)
else()
  message(STATUS "Found slangc: ${SLANGC_EXECUTABLE}")
endif()

function(_compile_slang_file)
  set(SINGLEVALUE SOURCE OUT TARGET)
  set(MULTIVALUES ENTRIES)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "${SINGLEVALUE}" "${MULTIVALUES}")
  message(STATUS "Compiling shader source file: ${arg_SOURCE} to ${arg_OUT} for entries \"${arg_ENTRIES}\"")

  if(${arg_TARGET} STREQUAL GLSL)
    set(COMMAND_TARGET -target glsl -profile glsl_460)
  else()
    set(COMMAND_TARGET -target spirv -profile spirv_1_4 -emit-spirv-directly)
  endif()


  set(ENTRIES)
  foreach(entry ${arg_ENTRIES})
    set(ENTRIES ${ENTRIES} -entry ${entry})
  endforeach()
  add_custom_command(
    OUTPUT ${OUT_FILE}
    DEPENDS ${source}.slang ${INCLUDED_FILES}
    COMMAND ${SLANGC_EXECUTABLE} ${arg_SOURCE} ${COMMAND_TARGET} -fvk-use-entrypoint-name ${ENTRIES} -o ${arg_OUT}
    COMMENT "Compiling shader ${source} to ${arg_TARGET} (${arg_OUT})"
    VERBATIM
  )
endfunction()


function(compile_shader target shader_target link_target)
  set(MULTIVALUE SOURCES INCLUDES)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "${MULTIVALUE}")

  set(VALID_OUTPUT_TARGETS GLSL SPIRV)

  if(NOT shader_target IN_LIST VALID_OUTPUT_TARGETS)
    message(FATAL_ERROR "Invalid output target: ${shader_target}. Valid targets are: ${VALID_OUTPUT_TARGETS}")
  endif()

  
  set(OUTPUTS "")
  set(INCLUDED_FILES "")

  message(STATUS "Including ${arg_INCLUDES} in shader compilation")

  foreach(file ${arg_INCLUDES})
    set(INCLUDED_FILES ${INCLUDED_FILES} include/${file}.slang)
  endforeach()

  message(STATUS "Included files: ${INCLUDED_FILES}")

  foreach(source ${arg_SOURCES})
    set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${source}.slang)
    message(STATUS "Processing shader source: ${SOURCE_FILE}")

    if(${shader_target} STREQUAL SPIRV)
      set(OUT_FILE ${CMAKE_BINARY_DIR}/shaders/${source}.spv)
      _compile_slang_file(SOURCE ${SOURCE_FILE} TARGET ${shader_target} ENTRIES vert frag OUT ${OUT_FILE})
      set(OUTPUTS ${OUTPUTS} ${OUT_FILE})
    else()
      foreach(stage vert frag)
        set(OUT_FILE ${CMAKE_BINARY_DIR}/shaders/${source}_${stage}.glsl)
        _compile_slang_file(SOURCE ${SOURCE_FILE} TARGET ${shader_target} ENTRIES "${stage}" OUT ${OUT_FILE})
        set(OUTPUTS ${OUTPUTS} ${OUT_FILE})
      endforeach()
    endif()
  endforeach()

  message(STATUS "Shader compilation outputs: ${OUTPUTS}")

  add_custom_target(${target} ALL
    DEPENDS ${OUTPUTS}
    COMMENT "Compiling shaders for target ${target}"
  )

  add_dependencies(${link_target} ${target})
endfunction()

function(copy_shaders target)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/shaders $<TARGET_FILE_DIR:${target}>/shaders
    COMMENT "Copying shaders to output directory"
  )
endfunction()