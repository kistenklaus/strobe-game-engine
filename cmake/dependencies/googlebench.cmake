include_guard(GLOBAL)

function(require_googlebenchmark)
  set(FORCE FALSE)
  if (DEFINED ARGV0)
    set(FORCE ${ARGV0})
  endif()

  # Always call find_package so imported targets are defined if available.
  find_package(benchmark QUIET CONFIG)

  if (TARGET benchmark::benchmark)
    set(GOOGLEBENCHMARK_FOUND TRUE CACHE INTERNAL "Google Benchmark found status")
  else()
    set(GOOGLEBENCHMARK_FOUND FALSE CACHE INTERNAL "Google Benchmark found status")
  endif()

  # If not found, fetch it.
  if (NOT GOOGLEBENCHMARK_FOUND)
    if (FORCE)
      include(FetchContent)

      # Avoid building Google Benchmark's own tests/install rules as part of this project.
      set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
      set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
      set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

      FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.4
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
      )

      FetchContent_MakeAvailable(googlebenchmark)

      if (TARGET benchmark::benchmark)
        set(GOOGLEBENCHMARK_FOUND TRUE CACHE INTERNAL "Google Benchmark was fetched and built")
      else()
        set(GOOGLEBENCHMARK_FOUND FALSE CACHE INTERNAL "Google Benchmark fetch failed")
      endif()
    else()
      set(GOOGLEBENCHMARK_FOUND FALSE CACHE INTERNAL "Google Benchmark not available")
    endif()
  endif()

  # Check if we've already logged.
  if (NOT DEFINED GOOGLEBENCHMARK_LOGGED)
    if (GOOGLEBENCHMARK_FOUND)
      log_success("✅ Google Benchmark available")
    elseif(FORCE)
      log_error("❌ Google Benchmark not available, but it was required!")
    else()
      log_warn("⚠️ Google Benchmark not available (optional)")
    endif()

    set(GOOGLEBENCHMARK_LOGGED TRUE CACHE INTERNAL "Google Benchmark logging done")
  endif()
endfunction()

# Link Google Benchmark without providing main().
function(target_link_googlebenchmark target visibility)
  require_googlebenchmark(TRUE)
  target_link_libraries(${target} ${visibility} benchmark::benchmark)
endfunction()

# Link Google Benchmark with benchmark-provided main().
function(target_link_googlebenchmark_main target visibility)
  require_googlebenchmark(TRUE)
  target_link_libraries(${target} ${visibility} benchmark::benchmark_main)
endfunction()
