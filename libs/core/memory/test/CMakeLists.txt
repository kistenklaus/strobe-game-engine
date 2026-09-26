enable_testing()

add_executable(strobe-core-memory-test EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/align.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/arc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/allocator_ref.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/buddy_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/box.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mallocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mpmc_monotonic_pool_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/monotonic_pool_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/monotonic_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mpsc_monotonic_pool_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/page_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/poly_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ranked_mpsc_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ranked_mpmc_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ranked_sync_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/stl_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sync_monotonic_pool_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sync_monotonic_resource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sync_resource.cpp
)

target_link_googletest(strobe-core-memory-test PRIVATE)
target_link_libraries(strobe-core-memory-test 
  PRIVATE strobe-core-memory
)

strobe_discover_tests(strobe-core-memory-test)
