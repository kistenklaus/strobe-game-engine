enable_testing()

add_executable(strobe-core-memory-test EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/align.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/allocator_ref.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/buddy_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mallocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/page_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/stl_allocator.cpp
)

target_link_googletest(strobe-core-memory-test PRIVATE)
target_link_libraries(strobe-core-memory-test 
  PRIVATE strobe-core-memory
)

strobe_discover_tests(strobe-core-memory-test)
