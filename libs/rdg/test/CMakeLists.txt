add_executable(strobe-rdg-test EXCLUDE_FROM_ALL
  main.cpp
  rdg.cpp
)

target_link_googletest(strobe-rdg-test PRIVATE)
target_link_libraries(strobe-rdg-test PRIVATE strobe::rdg)

strobe_discover_tests(strobe-rdg-test)
