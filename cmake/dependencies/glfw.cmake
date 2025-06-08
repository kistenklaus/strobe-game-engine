include_guard(GLOBAL)

function(require_glfw)
  set(FORCE FALSE)
  if (DEFINED ARGV0)
    set(FORCE ${ARGV0})
  endif()

  if (DEFINED GLFW_FOUND AND GLFW_FOUND)
    return()
  endif()

  # Try to find system GLFW
  if (FORCE)
    find_package(glfw3 REQUIRED)
  else()
    find_package(glfw3 QUIET)
  endif()

  set(GLFW_FOUND ${glfw3_FOUND} CACHE INTERNAL "GLFW found status")

  if (NOT GLFW_FOUND)
    # Check for X11
    find_package(X11 QUIET)
    if (X11_FOUND)
      set(GLFW_BUILD_X11 ON CACHE BOOL "Enable X11 backend")
    else()
      set(GLFW_BUILD_X11 OFF CACHE BOOL "Disable X11 backend")
    endif()

    # Check for Wayland
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
      pkg_check_modules(WAYLAND wayland-client>=0.2.7 wayland-egl wayland-cursor)
      pkg_check_modules(XKBCOMMON xkbcommon)
    endif()

    if (WAYLAND_FOUND AND XKBCOMMON_FOUND)
      set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Enable Wayland backend")
    else()
      set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Disable Wayland backend")
    endif()
    # Fallback: FetchContent
    include(FetchContent)
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG 43c9fb329116fcd57e94c8edc9ce2c1619a370a8
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(glfw)
    set(GLFW_FOUND TRUE CACHE INTERNAL "GLFW was fetched and built")
  endif()

  if (GLFW_FOUND)
    log_success("✅ GLFW available")
  elseif(FORCE)
    log_error("❌ GLFW not available, but it was required!")
  else()
    log_warn("⚠️ GLFW not available (optional)")
  endif()
endfunction()

function(target_link_glfw target visibility)
  require_glfw(TRUE)
  target_link_libraries(${target} ${visibility} glfw)

  # Add the include directory from the glfw target itself
  # get_target_property(GLFW_INCLUDE_DIR glfw INTERFACE_INCLUDE_DIRECTORIES)
  # if (GLFW_INCLUDE_DIR)
  #   target_include_directories(${target} ${visibility} ${GLFW_INCLUDE_DIR})
  # endif()
endfunction()
