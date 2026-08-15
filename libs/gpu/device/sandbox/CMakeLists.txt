


add_executable(strobe-device-sandbox 
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(strobe-device-sandbox
  PRIVATE 
    strobe-gpu-vulkan
    strobe-gpu-device
    strobe-platform
    strobe-window
    strobe::tracy
)

