set(STROBE_CORE_EVENTS_TESTS ${STROBE_CORE_EVENTS_LIB}-test)

enable_testing()

add_executable(${STROBE_CORE_EVENTS_TESTS} 
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/EventDispatcher.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/LockFreeMPSCRingBuffer.cpp
)

target_compile_options(${STROBE_CORE_EVENTS_TESTS} PUBLIC -g -O0)

target_link_googletest(${STROBE_CORE_EVENTS_TESTS} PUBLIC)

target_link_libraries(${STROBE_CORE_EVENTS_TESTS} PUBLIC ${STROBE_CORE_EVENTS_LIB})
target_link_libraries(${STROBE_CORE_EVENTS_TESTS} PUBLIC ${STROBE_CORE_CONTAINERS_LIB})

