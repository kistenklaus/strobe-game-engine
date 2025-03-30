include_guard(GLOBAL)

include(cmake/colorful.cmake)
include(FetchContent)

# Try to find spdlog via system package first
find_package(spdlog QUIET)

if (TARGET spdlog::spdlog)
  log_success("✅ spdlog available (find_package)")
else()
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.12.0
  )
  FetchContent_MakeAvailable(spdlog)

  if (TARGET spdlog::spdlog)
    log_success("✅ spdlog available (FetchContent)")
  else()
    log_error("❌ Failed to find or fetch spdlog library")
  endif()
endif()

# Link if found
if (TARGET spdlog::spdlog)
  target_link_libraries(${EXECUTABLE} PRIVATE spdlog::spdlog)
endif()
