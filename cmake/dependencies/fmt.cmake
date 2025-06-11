include_guard(GLOBAL)

function(fmt_found_log)
  if (NOT GLFW_FMT_LOG)
    log_success("✅ fmt available")
  endif()
  set(GLFW_FMT_LOG TRUE CACHE INTERNAL "")
endfunction()

function(require_fmt)
  set(FORCE FALSE)
  if (DEFINED ARGV0)
    set(FORCE ${ARGV0})
  endif()

  # Try to find system fmt
  find_package(fmt QUIET)

  if (NOT fmt_FOUND)
    # Fetch GLFW
    include(FetchContent)
    FetchContent_Declare(
      fmt
      GIT_REPOSITORY https://github.com/fmtlib/fmt.git
      GIT_TAG 40626af88bd7df9a5fb80be7b25ac85b122d6c21
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(fmt)
    set(fmt_FOUND TRUE)
  endif()

  if (fmt_FOUND)
    glfw_found_log()
  elseif(FORCE)
    log_error("❌ fmt not available, but it was required!")
    set(FMT_FOUND_LOG FALSE CACHE INTERNAL "")
  else()
    log_warn("⚠️ fmt not available (optional)")
    set(FMT_FOUND_LOG FALSE CACHE INTERNAL "")
  endif()

endfunction()


function(target_link_fmt target visibility)
  require_fmt(TRUE)
  target_link_libraries(${target} ${visibility} fmt)

  # Add the include directory from the glfw target itself
  # get_target_property(GLFW_INCLUDE_DIR glfw INTERFACE_INCLUDE_DIRECTORIES)
  # if (GLFW_INCLUDE_DIR)
  #   target_include_directories(${target} ${visibility} ${GLFW_INCLUDE_DIR})
  # endif()
endfunction()
