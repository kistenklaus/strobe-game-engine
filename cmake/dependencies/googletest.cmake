include_guard(GLOBAL)


function(require_googletest)
  set(FORCE FALSE)
  if (DEFINED ARGV0)
    set(FORCE ${ARGV0})
  endif()

  # Always call find_package so that imported targets are defined
  find_package(GTest QUIET)

  # Update the cached result
  set(GTEST_FOUND ${GTest_FOUND} CACHE INTERNAL "GoogleTest found status")

  # If not found, fetch it
  if (NOT GTEST_FOUND)
    if (FORCE)
      include(FetchContent)
      FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      )
      FetchContent_MakeAvailable(googletest)
      set(GTEST_FOUND TRUE CACHE INTERNAL "GoogleTest was fetched and built")
    else()
      set(GTEST_FOUND FALSE CACHE INTERNAL "GoogleTest not available")
    endif()
  endif()

  # Check if we've already logged
  if (NOT DEFINED GTEST_LOGGED)
    if (GTEST_FOUND)
      log_success("✅ GoogleTest available")
    elseif(FORCE)
      log_error("❌ GoogleTest not available, but it was required!")
    else()
      log_warn("⚠️ GoogleTest not available (optional)")
    endif()
    set(GTEST_LOGGED TRUE CACHE INTERNAL "GoogleTest logging done")
  endif()
endfunction()

# Function to link GoogleTest to a target
function(target_link_googletest target visibility)
  # Ensure GoogleTest is found (required in this context)
  require_googletest(TRUE)
  target_link_libraries(${target} ${visibility} GTest::gtest)
endfunction()
