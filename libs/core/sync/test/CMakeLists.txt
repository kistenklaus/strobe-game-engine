set(STROBE_CORE_SYNC_TESTS ${STROBE_CORE_SYNC_LIB}-test)

enable_testing()

add_executable(${STROBE_CORE_SYNC_TESTS} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/LockFreeSPSCRingBuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/LockFreeBoundedMSQueue.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/mpsc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/spsc.cpp
)

target_compile_options(${STROBE_CORE_SYNC_TESTS} PUBLIC -g -O0)

target_link_googletest(${STROBE_CORE_SYNC_TESTS} PUBLIC)
target_link_libraries(${STROBE_CORE_SYNC_TESTS} PRIVATE ${STROBE_CORE_SYNC_LIB})


strobe_discover_tests(${STROBE_CORE_SYNC_TESTS})

