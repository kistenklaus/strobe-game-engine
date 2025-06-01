include_guard(GLOBAL)

include(cmake/colorful.cmake)
include(FetchContent)

if (NOT TARGET SPIRV-Headers)
  FetchContent_Declare(
    spirv_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG        main
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(spirv_headers)

  if (TARGET SPIRV-Headers)
    log_success("✅ SPIRV-Headers available (FetchContent)")
  else()
    log_error("❌ SPIRV-Headers not available")
  endif()
endif()
