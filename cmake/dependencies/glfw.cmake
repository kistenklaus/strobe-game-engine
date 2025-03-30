include_guard(GLOBAL)

include(cmake/colorful.cmake)

find_package(glfw3 REQUIRED)

if (glfw3_FOUND)
  log_success("✅ GLFW available")
else()
  log_error("❌ GLFW not available!")
endif()

target_link_libraries(${EXECUTABLE} PRIVATE glfw)
