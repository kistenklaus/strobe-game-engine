add_executable(strobe-rdg-test EXCLUDE_FROM_ALL
  main.cpp
  barrier_compiler_test.cpp
  materializer_test.cpp
  mock_swapchain_lib.cpp
  mock_swapchain_lib_test.cpp
)

target_link_googletest(strobe-rdg-test PRIVATE)
target_link_libraries(strobe-rdg-test PRIVATE strobe::rdg)

target_include_directories(strobe-rdg-test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../sandbox
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/strobe/rdg/detail/compiler
)

strobe_discover_tests(strobe-rdg-test)
