add_executable(strobe-core-graph-test EXCLUDE_FROM_ALL
  algorithms.cpp
  graph.cpp
  main.cpp
)

target_link_googletest(strobe-core-graph-test PRIVATE)
target_link_libraries(strobe-core-graph-test PRIVATE strobe-core-graph)

strobe_discover_tests(strobe-core-graph-test)
