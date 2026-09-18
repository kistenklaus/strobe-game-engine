include_guard(GLOBAL)

add_library(strobe-imgui-platform EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/platform.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/imgui/platform/keyboard.cpp
)

target_include_directories(strobe-imgui-platform
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(strobe-imgui-platform
  PUBLIC strobe-core
  PUBLIC imgui
  PUBLIC strobe-platform
)
