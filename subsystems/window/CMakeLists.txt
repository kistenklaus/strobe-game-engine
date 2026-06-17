include_guard(GLOBAL)

set(STROBE_WINDOW_SUBSYS ${PROJECT_NAME}-window CACHE INTERNAL "")

add_library(${STROBE_WINDOW_SUBSYS} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/window/window_impl.cpp
)

target_include_directories(${STROBE_WINDOW_SUBSYS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_WINDOW_SUBSYS} 
  PUBLIC ${STROBE_CORE_LIB}
  PUBLIC ${STROBE_PLATFORM}
)

target_link_glfw(${STROBE_WINDOW_SUBSYS} PUBLIC)
