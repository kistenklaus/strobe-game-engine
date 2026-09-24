enable_testing()

add_executable(strobe-core-container-tests EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/bitmap_index_pool.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/bitset.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bitvec.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/boxed_array.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/boxed_bitset.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/boxed_str.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hash_map.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inplace_bitvec.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inplace_linear_map.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inplace_linear_set.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inplace_string.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inplace_vector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inplace_vector_deque.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/linear_map.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/linear_set.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/small_bitvec.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/small_linear_map.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/small_linear_set.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/small_string.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/small_vector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/small_vector_deque.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/span.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/str.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/string.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vector_deque.cpp
)

target_link_googletest(strobe-core-container-tests PRIVATE)
target_link_libraries(strobe-core-container-tests
  PRIVATE strobe-core-container
)

strobe_discover_tests(strobe-core-container-tests) 
