set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "Disable exceptions in spdlog" FORCE)
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.3)
FetchContent_MakeAvailable(spdlog)

function(link_spdlog_header_only TARGET ACCESS)
  target_link_libraries(${TARGET} ${ACCESS} spdlog::spdlog)
endfunction()