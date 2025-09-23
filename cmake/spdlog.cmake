function(set_spdlog_no_except VALUE)
  if(VALUE)
    set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "Disable exceptions in spdlog" FORCE)
  else()
    set(SPDLOG_NO_EXCEPTIONS OFF CACHE BOOL "Enable exceptions in spdlog" FORCE)
  endif()
endfunction()

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.3)
FetchContent_MakeAvailable(spdlog)

function(link_spdlog TARGET ACCESS)
  target_link_libraries(${TARGET} ${ACCESS} spdlog::spdlog)
endfunction()