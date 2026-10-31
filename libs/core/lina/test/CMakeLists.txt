enable_testing()

add_executable(strobe-core-lina-test EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/quaternion.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vec.cpp
)

target_link_googletest(strobe-core-lina-test PRIVATE)
target_link_libraries(strobe-core-lina-test
  PRIVATE strobe-core-lina
)

strobe_discover_tests(strobe-core-lina-test)
