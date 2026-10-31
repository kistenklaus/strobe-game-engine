enable_testing()

add_executable(strobe-core-color-test EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_googletest(strobe-core-color-test PRIVATE)
target_link_libraries(strobe-core-color-test
  PRIVATE strobe-core-color
)

strobe_discover_tests(strobe-core-color-test) 
