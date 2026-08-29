include_guard(GLOBAL)

# Vulkan RHI:
# sub components listed in dependency order
# Later components may only list eariler components.
add_library(strobe-rhi-vulkan EXCLUDE_FROM_ALL
  # strobe::rhi::vulkan
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/cmd/barrier.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/cmd/rendering.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/cmd/transfer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/debug_utils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/get_queues.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/instance.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/logical_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/select_physical_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/select_queues.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/context/vma.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/device_info/device_extensions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/device_info/device_features.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/device_info/device_properties.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/device_info/memory_properties.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/device_info/queue_family_properties.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/acceleration_structure.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/binary_semaphore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/command_buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/command_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/debug_name.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/event.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/fence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/image_view.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/memory_usage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/query_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/queue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/shader_object.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/surface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/swapchain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/vulkan/timeline_semaphore.cpp
  # strobe::rhi context
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/context/context.cpp
  # strobe::rhi shader
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/shader/compute_shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/shader/fragment_shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/shader/shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/shader/vertex_shader.cpp
  # strobe::rhi sync
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/sync/binary_semaphore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/sync/fence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/sync/sync.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/sync/timeline_semaphore.cpp
  # strobe::rhi mem
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/memory/memory_allocation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/memory/memory_allocation_impl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/memory/memory_lifetime_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/memory/memory_pool.cpp
  # strobe::rhi buf
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/buf/buffer.cpp
  # strobe::rhi img
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/img/image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/img/image_view.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/img/img.cpp
  # strobe::rhi stage
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/stage/stage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/stage/staging_pool.cpp
  # strobe::rhi bvh
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/bvh/blas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/bvh/bvh.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/bvh/scratch_buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/bvh/tlas.cpp
  # strobe::rhi command
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/cmd/cmd.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/cmd/command_buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/cmd/command_buffer_impl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/cmd/command_pool.cpp
  # strobe::rhi gc
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/gc/garbage_collector.cpp
  

  # #strobe::rhi swapchain
  # ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/swapchain/surface.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/swapchain/swapchain.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/swapchain/swapchain_generation.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/swapchain/swapchain_image.cpp
  # # strobe::rhi device
  # ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/device/device.cpp
  # #strobe::rhi queue
  # ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/rhi/queue/queue.cpp


)
target_include_directories(strobe-rhi-vulkan
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan
)
target_link_libraries(strobe-rhi-vulkan
  PRIVATE strobe-core
  PRIVATE Vulkan::Vulkan
  PRIVATE strobe::vma
)
target_link_glfw(strobe-rhi-vulkan PRIVATE)

# Select RHI implementation:
add_library(strobe::rhi ALIAS strobe-rhi-vulkan)

# Sandbox
add_executable(strobe-rhi-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)
target_link_libraries(strobe-rhi-sandbox
  PRIVATE strobe::rhi
  PRIVATE strobe-platform
  PRIVATE strobe-window
  PRIVATE strobe::tracy

  PRIVATE Vulkan::Vulkan
  PRIVATE strobe::vma
)
target_include_directories(strobe-rhi-sandbox
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan
)
