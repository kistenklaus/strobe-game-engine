include_guard(GLOBAL)


add_subdirectory(renderer)


#
# add_library(strobe-imgui EXCLUDE_FROM_ALL
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_context.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_frame.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_render.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_samplers.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_shaders.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_texture.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/imgui_viewport.cpp
# )
# target_include_directories(strobe-imgui
#   PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
#   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
# )
# target_link_libraries(strobe-imgui
#   PUBLIC strobe::rhi
#   PUBLIC imgui
#   PUBLIC imgui-glfw-backend
#   PRIVATE strobe-imgui-vertex-shader
#   PRIVATE strobe-imgui-fragment-shader
# )

# ====== Sandbox ==========
add_executable(strobe-imgui-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)

target_link_libraries(strobe-imgui-sandbox
  PRIVATE strobe-imgui-renderer
  PRIVATE strobe-window
  PRIVATE strobe-platform
  PRIVATE imgui-glfw-backend
)

target_include_directories(strobe-imgui-sandbox
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

