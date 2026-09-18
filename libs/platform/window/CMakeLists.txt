include_guard(GLOBAL)

add_library(strobe-platform-window EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/platform/window.cpp
)

target_include_directories(strobe-platform-window
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(strobe-platform-window
  PUBLIC strobe-core
  PUBLIC strobe-platform
)

target_link_glfw(strobe-platform-window PRIVATE)


add_executable(strobe-platform-window-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)

target_link_libraries(strobe-platform-window-sandbox
  PRIVATE strobe-platform-window
)

target_include_directories(strobe-platform-window-sandbox
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_fmt(strobe-platform-window-sandbox PRIVATE)




