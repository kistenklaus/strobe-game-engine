enable_testing()

add_executable(strobe-core-memory-bench EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_googlebenchmark(strobe-core-memory-bench PUBLIC)

target_link_libraries(strobe-core-memory-bench
  PRIVATE strobe-core-memory
)

target_include_directories(strobe-core-memory-bench
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
