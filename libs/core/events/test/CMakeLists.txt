enable_testing()

add_executable(strobe-core-events-test EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/event_dispatcher.cpp
)

target_link_googletest(strobe-core-events-test PUBLIC)

target_link_libraries(strobe-core-events-test
  PUBLIC strobe-core-events
  PUBLIC strobe-core-containers
)

strobe_discover_tests(strobe-core-events-test)

