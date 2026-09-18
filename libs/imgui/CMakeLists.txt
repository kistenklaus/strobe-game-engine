include_guard(GLOBAL)


add_subdirectory(renderer)
add_subdirectory(platform)


# ====== Sandbox ==========
add_executable(strobe-imgui-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)

target_link_libraries(strobe-imgui-sandbox
  PRIVATE strobe-imgui-renderer
  PRIVATE strobe-imgui-platform
  PRIVATE strobe-platform
)
