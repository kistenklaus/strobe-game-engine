include_guard(GLOBAL)

# Function to require GoogleTest (optional 'force' argument, default: FALSE)
function(require_googletest)
  # Optional 'force' argument (default is FALSE)
  set(FORCE FALSE)
  if (DEFINED ARGV0)
    set(FORCE ${ARGV0})
  endif()

  # Check if GoogleTest has already been found (using a cache variable)
  if (DEFINED GTEST_FOUND AND GTEST_FOUND)
    return()
  endif()

  # Attempt to find GoogleTest (only once)
  if (FORCE)
    find_package(GTest REQUIRED)
  else()
    find_package(GTest QUIET)
  endif()

  # If GoogleTest is not found, download and build it
  if (NOT GTEST_FOUND)
    if (FORCE)
      include(FetchContent)
      FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
      )
      FetchContent_MakeAvailable(googletest)
      set(GTEST_FOUND TRUE CACHE INTERNAL "GoogleTest was fetched and built")
    else()
      set(GTEST_FOUND FALSE CACHE INTERNAL "GoogleTest not available")
    endif()
  endif()

  # Log result
  if (GTEST_FOUND)
    log_success("✅ GoogleTest available")
  elseif(FORCE)
    log_error("❌ GoogleTest not available, but it was required!")
  else()
    log_warn("⚠️ GoogleTest not available (optional)")
  endif()
endfunction()

# Function to link GoogleTest to a target
function(target_link_googletest target visibility)
  # Ensure GoogleTest is found (required in this context)
  require_googletest(TRUE)
  target_link_libraries(${target} ${visibility} GTest::gtest)
endfunction()
