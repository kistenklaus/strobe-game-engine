include_guard(GLOBAL)

set(STROBE_INPUT_SUBSYS ${PROJECT_NAME}-input CACHE INTERNAL "")

add_library(${STROBE_INPUT_SUBSYS} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/input/InputSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/input/Keyboard.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/input/Mouse.cpp
)
target_include_directories(${STROBE_INPUT_SUBSYS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_INPUT_SUBSYS} PUBLIC ${STROBE_WINDOW_SUBSYS})

if (STROBE_BUILD_SANDBOXES)
  add_subdirectory(sandbox)
endif()
