


add_executable(strobe-device-sandbox 
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(strobe-device-sandbox
  PRIVATE 
    strobe-gpu-vulkan
    strobe-platform
    strobe-window
)

