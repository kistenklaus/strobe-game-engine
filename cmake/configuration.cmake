include_guard(GLOBAL)


function (select_strobe_window_api)
    if (DEFINED ARGV0)
      set(api ${ARGV0})
      if (NOT api STREQUAL "GLFW")
        log_error("${api} is not a valid window api. We only support GLFW for now.")
      else() 
        set(STROBE_WINDOW_API ${api} CACHE INTERNAL "Selected strobe window api")
      endif()
    endif()

    require_glfw(FALSE)
    if (GLFW_FOUND)
      log_success("✅ Selected GLFW as the window api.")
      set(STROBE_WINDOW_API GLFW CACHE INTERNAL "Selected strobe window api")
    else()
      log_error("❌ Failed to select window api!")
    endif()
    # we will discuss the section later
endfunction()
