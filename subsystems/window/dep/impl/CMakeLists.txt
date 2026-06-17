
set(STROBE_WINDOW_IMPL ${STROBE_WINDOW_SUBSYS}-impl CACHE INTERNAL "Strobe window implementationt target")

add_library(${STROBE_WINDOW_IMPL} INTERFACE)


target_link_libraries(${STROBE_WINDOW_IMPL} INTERFACE ${STROBE_CORE_LIB})
target_link_libraries(${STROBE_WINDOW_IMPL} INTERFACE ${STROBE_WINDOW_COMMON})
target_include_directories(${STROBE_WINDOW_IMPL} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (STROBE_WINDOW_API STREQUAL "GLFW")
  add_subdirectory(glfw)
  target_link_libraries(${STROBE_WINDOW_IMPL} INTERFACE ${STROBE_WINDOW_GLFW_IMPL})
else()
  log_error("No library found!!")
endif()

