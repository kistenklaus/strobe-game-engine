include_guard(GLOBAL)

set(STROBE_GPU_DEVICE_LIB ${STROBE_GPU_LIB}-device)


set(VK_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/binary_semaphore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/blas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/command_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/command_buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/command_buffer_impl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/compute_shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/descriptor_heap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/fence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/fragment_shader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/image_view.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/memory_allocation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/memory_allocation_impl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/memory_lifetime_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/memory_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/queue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/surface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/swapchain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/swapchain_generation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/swapchain_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/timeline_semaphore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/vertex_shader.cpp
)
set(VK_INCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan
)

set(SRCS ${VK_SRCS})
set(PRIVATE_INC ${VK_INCLUDE})

add_library(${STROBE_GPU_DEVICE_LIB} EXCLUDE_FROM_ALL
  ${SRCS}
)

target_include_directories(${STROBE_GPU_DEVICE_LIB}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${PRIVATE_INC} # tmp
)


target_link_libraries(${STROBE_GPU_DEVICE_LIB}
  PUBLIC ${STROBE_CORE_LIB}
  PRIVATE ${STROBE_GPU_LIB}-vulkan
  PRIVATE strobe::tracy
)


if(STROBE_WARNINGS_AS_ERRORS)
  target_compile_options(${STROBE_GPU_DEVICE_LIB} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
  )
endif()


if (STROBE_SAN)
  target_link_libraries(${STROBE_GPU_DEVICE_LIB} PRIVATE strobe_sanitizers)
endif()

add_subdirectory(sandbox)

