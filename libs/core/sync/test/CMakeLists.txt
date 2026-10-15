add_executable(strobe-core-sync-test EXCLUDE_FROM_ALL worker_pool.cpp)

target_link_googletest(strobe-core-sync-test PRIVATE)
target_link_libraries(strobe-core-sync-test PRIVATE strobe-core-sync)

strobe_discover_tests(strobe-core-sync-test)
