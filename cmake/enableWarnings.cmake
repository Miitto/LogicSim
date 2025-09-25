function(enable_warnings TARGET)
  target_compile_options(${TARGET} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd5050 /wd4458 /wd4127>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token>
  )
endfunction()