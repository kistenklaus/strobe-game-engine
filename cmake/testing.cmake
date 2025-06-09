include_guard(GLOBAL)  # or DIRECTORY

if (STROBE_BUILD_TESTS)
  enable_testing()
  add_custom_target(build-tests)
endif()

function(strobe_discover_tests test_target)

  if (STROBE_BUILD_TESTS)
    # 1. Call gtest_discover_tests with PRE_TEST discovery
    gtest_discover_tests(${test_target} DISCOVERY_MODE PRE_TEST)

    # 2. Register test_target as a dependency of build-tests
    add_dependencies(build-tests ${test_target})
  endif()
endfunction()


add_custom_target(run-tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS build-tests
  COMMENT "Building and running all tests"
)
