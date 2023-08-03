
function(create_test_target TARGET_TO_TEST TEST_DIR)
  set(TEST_EXEC_NAME "${TARGET_TO_TEST}_test")

  set(ABS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR}")
  set(ABS_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_DIR}")

  add_executable(${TEST_EXEC_NAME} ${ABS_SRC_DIR})

  file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/**test.cpp")


  target_link_libraries(${TEST_EXEC_NAME} 
    PUBLIC GTest::gtest_main
    PRIVATE ${TARGET_TO_TEST})
  include(GoogleTest)

  gtest_discover_tests(${TEST_EXEC_NAME})

# add_test(NAME ${PIL_TEST_EXEC_NAME} COMMAND ${TEST_EXEC_NAME})

  add_custom_command(
       TARGET ${TEST_EXEC_NAME}
       COMMENT "Run tests"
       POST_BUILD 
       WORKING_DIRECTORY ${ABS_BIN_DIR}
       COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^${PIL_TEST_EXEC_NAME}$" --output-on-failures
  )
endfunction()
