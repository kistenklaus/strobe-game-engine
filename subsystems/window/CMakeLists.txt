include_guard(GLOBAL)

set(STROBE_WINDOW_SUBSYS ${PROJECT_NAME}-window CACHE INTERNAL "")

add_subdirectory(common)
add_subdirectory(impl)


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_WINDOW_SUBSYS}
  ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)
target_include_directories(${STROBE_WINDOW_SUBSYS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_WINDOW_SUBSYS} PUBLIC ${STROBE_WINDOW_COMMON})
target_link_libraries(${STROBE_WINDOW_SUBSYS} PUBLIC ${STROBE_WINDOW_IMPL})
target_link_libraries(${STROBE_WINDOW_SUBSYS} PUBLIC ${STROBE_CORE_LIB})

add_subdirectory(sandbox)
