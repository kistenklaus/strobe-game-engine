
enable_testing()

add_executable(strobe-core-container-bench EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_googlebenchmark(strobe-core-container-bench PUBLIC)

# target_compile_options(strobe-core-container-bench PRIVATE -mavx2)

target_link_libraries(strobe-core-container-bench
  PRIVATE strobe-core-container
)

target_include_directories(strobe-core-container-bench
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)


