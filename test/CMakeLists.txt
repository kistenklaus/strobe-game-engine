
get_property(TEST_LIBS GLOBAL PROPERTY STROBE_TEST_LIBS)

set(TEST_SUITE_NAME ${PROJECT_NAME}-test)

# Create a list of object file references
set(TEST_OBJECTS)
foreach(test_lib IN LISTS TEST_LIBS)
  list(APPEND TEST_OBJECTS $<TARGET_OBJECTS:${test_lib}>)
endforeach()

# Create the unified test executable
add_executable(${TEST_SUITE_NAME} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${TEST_OBJECTS}
)

target_link_googletest(${TEST_SUITE_NAME} PRIVATE)

require_googletest(TRUE)

enable_testing()

gtest_discover_tests(${TEST_SUITE_NAME})

add_custom_target(test
  COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS ${TEST_SUITE_NAME}
  COMMENT "Building and running all tests"
)
