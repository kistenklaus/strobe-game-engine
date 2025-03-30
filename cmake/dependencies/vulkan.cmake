include_guard(GLOBAL)  # or DIRECTORY

include(cmake/colorful.cmake)

find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator)

if (Vulkan_FOUND)
  log_success("✅ Vulkan available: ${Vulkan_LIBRARY}")
else()
  log_error("❌ Vulkan not available!")
endif()



target_include_directories(${EXECUTABLE}
  PUBLIC
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${EXECUTABLE} PRIVATE Vulkan::Vulkan)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    GLFW_INCLUDE_VULKAN
)
