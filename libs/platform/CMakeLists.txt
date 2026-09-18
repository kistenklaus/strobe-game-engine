include_guard(GLOBAL)

add_library(strobe-platform EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/platform/monitor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/platform/platform.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/platform/window.cpp
)

target_include_directories(strobe-platform
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(strobe-platform
  PUBLIC strobe-core
)
target_link_glfw(strobe-platform PRIVATE)

set_property(
    TARGET strobe-platform
    APPEND PROPERTY INTERFACE_LINK_OPTIONS
    "LINKER:--wrap=main"
)

add_executable(strobe-platform-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)

target_link_libraries(strobe-platform-sandbox 
  PRIVATE strobe-platform
)
