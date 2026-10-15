add_executable(strobe-flow-test EXCLUDE_FROM_ALL
  compiler.cpp
  op_stream.cpp
  plugins.cpp
  program.cpp
  scenarios.cpp
)

target_link_googletest(strobe-flow-test PRIVATE)
target_link_libraries(strobe-flow-test PRIVATE strobe::flow)
target_include_directories(strobe-flow-test PRIVATE ${PROJECT_SOURCE_DIR}/libs/flow/src)

strobe_discover_tests(strobe-flow-test)
