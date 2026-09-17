


add_library(imgui STATIC EXCLUDE_FROM_ALL
  ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui.cpp
  ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
  ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
  ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
  ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
)

target_include_directories(imgui 
  PUBLIC ${PROJECT_SOURCE_DIR}/third_party/imgui
)

add_library(imgui-glfw-backend STATIC EXCLUDE_FROM_ALL
  ${PROJECT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui-glfw-backend 
  PUBLIC ${PROJECT_SOURCE_DIR}/third_party/imgui/backends
)
target_link_libraries(imgui-glfw-backend PUBLIC imgui)

