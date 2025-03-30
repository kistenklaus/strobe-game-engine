include_guard(GLOBAL)

include(cmake/colorful.cmake)
include(FetchContent)

if (NOT TARGET glslang)
  FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG        main
  )
  FetchContent_MakeAvailable(glslang)

  if (TARGET glslang)
    log_success("✅ glslang available (FetchContent)")
  else()
    log_error("❌ glslang not available")
  endif()
endif()
