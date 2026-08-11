include_guard(GLOBAL)

set(STROBE_GPU_DEVICE_LIB ${STROBE_GPU_LIB}-device)


set(VK_SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/strobe/gpu/device/device.cpp
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
  PRIVATE ${PRIVATE_INC}
)


target_link_libraries(${STROBE_GPU_DEVICE_LIB} 
  PUBLIC ${STROBE_CORE_LIB}
  PRIVATE ${STROBE_GPU_LIB}-vulkan
)

add_subdirectory(sandbox)

