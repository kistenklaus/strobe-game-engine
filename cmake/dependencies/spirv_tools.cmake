include_guard(GLOBAL)

include(cmake/colorful.cmake)
include(FetchContent)

set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_TOOLS_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_TOOLS_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

# Dependency: SPIRV-Headers
include(cmake/dependencies/spirv_headers.cmake)

if (NOT TARGET SPIRV-Tools-opt)
  FetchContent_Declare(
    spirv_tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG        main
  )
  FetchContent_MakeAvailable(spirv_tools)

  if (TARGET SPIRV-Tools-opt)
    log_success("✅ SPIRV-Tools available (FetchContent)")
  else()
    log_error("❌ SPIRV-Tools not available")
  endif()
endif()

