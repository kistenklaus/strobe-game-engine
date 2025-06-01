include_guard(GLOBAL)

include(cmake/colorful.cmake)
include(FetchContent)


find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  log_success("PkgConfig found")
  pkg_check_modules(SHADERC IMPORTED_TARGET shaderc QUIET)

  if(SHADERC_FOUND)
    log_success("✅ shaderc available (pkg-config)")
    target_link_libraries(${EXECUTABLE} PRIVATE PkgConfig::SHADERC)
    return()
  endif()
endif()


# Fallback to FetchContent
log_info("ℹ️ Falling back to FetchContent for shaderc")

# Setup shaderc build options
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_STATIC ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)

# Required dependencies
include(cmake/dependencies/spirv_tools.cmake)
include(cmake/dependencies/glslang.cmake)

FetchContent_Declare(
  shaderc
  GIT_REPOSITORY https://github.com/google/shaderc.git
  GIT_TAG        main
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(shaderc)

if (TARGET shaderc_combined)
  log_success("✅ shaderc available (FetchContent)")
  target_link_libraries(${EXECUTABLE} PRIVATE shaderc_combined)
else()
  log_error("❌ shaderc not available")
endif()
