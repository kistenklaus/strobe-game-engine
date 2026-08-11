include_guard(GLOBAL)

set(STROBE_GPU_LIB ${PROJECT_NAME}-gpu)

add_subdirectory(vulkan)
add_subdirectory(device)

add_library(${STROBE_GPU_LIB} INTERFACE)

target_link_libraries(${STROBE_GPU_LIB}
  INTERFACE ${STROBE_GPU_VULKAN_LIB}
  INTERFACE ${STROBE_GPU_DEVICE_LIB}
)
