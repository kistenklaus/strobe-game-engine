
set(STROBE_WINDOW_IMPL ${STROBE_WINDOW_LIB}-impl CACHE INTERNAL "Strobe window implementationt target")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_WINDOW_IMPL} STATIC ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)


target_link_libraries(${STROBE_WINDOW_IMPL} PUBLIC ${STROBE_CORE_LIB})
target_include_directories(${STROBE_WINDOW_IMPL} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (STROBE_WINDOW_API STREQUAL "GLFW")
  add_subdirectory(glfw)
  target_link_libraries(${STROBE_WINDOW_IMPL} PRIVATE ${STROBE_WINDOW_GLFW_IMPL})
else()
  log_error("No library found!!")
endif()

