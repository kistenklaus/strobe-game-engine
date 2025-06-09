set(STROBE_CORE_MEMORY_TESTS ${STROBE_CORE_MEMORY_LIB}-test)


enable_testing()

add_executable(${STROBE_CORE_MEMORY_TESTS} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/MonotonicPoolResource.cpp
)

target_compile_options(${STROBE_CORE_MEMORY_TESTS} PUBLIC -g -O0)

target_link_googletest(${STROBE_CORE_MEMORY_TESTS} PRIVATE)

target_link_libraries(${STROBE_CORE_MEMORY_TESTS} PRIVATE ${STROBE_CORE_MEMORY_LIB})

strobe_discover_tests(${STROBE_CORE_MEMORY_TESTS}) 
