include_guard(GLOBAL)

# Function to require GLFW (optional 'force' argument, default: FALSE)
function(require_glfw)
  # Optional 'force' argument (default is FALSE)
  set(FORCE FALSE)
  if (DEFINED ARGV0)
    set(FORCE ${ARGV0})
  endif()

  # Check if GLFW has already been found (using a cache variable)
  if (DEFINED GLFW_FOUND AND GLFW_FOUND)
    return()
  endif()

  # Attempt to find GLFW (only once)
  if (FORCE)
    find_package(glfw3 REQUIRED)
  else()
    find_package(glfw3 QUIET)
  endif()

  # Cache the result of the search (so it is only done once)
  set(GLFW_FOUND ${glfw3_FOUND} CACHE INTERNAL "Tracks if GLFW has been found once")

  # Log result
  if (glfw3_FOUND)
    log_success("✅ GLFW available")
  elseif(FORCE)
    log_error("❌ GLFW not available, but it was required!")
  else()
    log_warn("⚠️ GLFW not available (optional)")
  endif()
endfunction()

# Function to link GLFW to a target
function(target_link_glfw target visibility)
  # Ensure GLFW is found (required in this context)
  require_glfw(TRUE)
  target_link_libraries(${target} ${visibility} glfw)
endfunction()
