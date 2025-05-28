set(STROBE_CORE_MEMORY_TESTS ${STROBE_CORE_MEMORY_LIB}-test)


enable_testing()

add_executable(${STROBE_CORE_MEMORY_TESTS} 
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/MonotonicPoolResource.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/LockFreeSPSCRingBuffer.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/LockFreeMPSCRingBuffer.cpp
)

target_compile_options(${STROBE_CORE_MEMORY_TESTS} PUBLIC -g -O0)

target_link_googletest(${STROBE_CORE_MEMORY_TESTS} PRIVATE)

target_link_libraries(${STROBE_CORE_MEMORY_TESTS} PRIVATE ${STROBE_CORE_MEMORY_LIB})

