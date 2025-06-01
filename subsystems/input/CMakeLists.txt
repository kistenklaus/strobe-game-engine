include_guard(GLOBAL)

set(STROBE_INPUT_SUBSYS ${PROJECT_NAME}-input CACHE INTERNAL "")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_INPUT_SUBSYS}
  ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/input/x.cpp
)
target_include_directories(${STROBE_INPUT_SUBSYS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_INPUT_SUBSYS} PUBLIC ${STROBE_WINDOW_SUBSYS})

add_subdirectory(sandbox)
