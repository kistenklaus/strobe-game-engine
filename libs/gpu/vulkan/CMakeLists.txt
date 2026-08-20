include_guard(GLOBAL)

set(STROBE_GPU_VULKAN_LIB ${STROBE_GPU_LIB}-vulkan)

option(INSTRUMENT_DRIVER OFF)
option(INSTRUMENT_VMA OFF)

add_library(${STROBE_GPU_VULKAN_LIB} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/cmd/barrier.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/debug_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/get_queues.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/instance.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/logical_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/select_physical_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/select_queues.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/surface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/context/vma.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/acceleration_structure.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/binary_semaphore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/command_buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/command_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/debug_name.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/descriptor_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/descriptor_set.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/descriptor_set_layout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/event.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/fence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/image_view.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/memory_usage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/query_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/queue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/sampler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/shader_module.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/shader_object.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/surface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/swapchain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/gpu/vulkan/timeline_semaphore.cpp
)

target_include_directories(${STROBE_GPU_VULKAN_LIB}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_GPU_VULKAN_LIB}
  PUBLIC ${STROBE_CORE_LIB}
  PUBLIC Vulkan::Vulkan
  PUBLIC strobe::tracy
  PUBLIC strobe::vma
)
target_link_glfw(${STROBE_GPU_VULKAN_LIB} PUBLIC)

if (INSTRUMENT_DRIVER)
  target_compile_definitions(${STROBE_GPU_VULKAN_LIB} PUBLIC STROBE_INSTRUMENT_VULKAN_DRIVER)
endif()

if (INSTRUMENT_VMA)
  target_compile_definitions(${STROBE_GPU_VULKAN_LIB} PUBLIC STROBE_INSTRUMENT_VMA)
endif()
