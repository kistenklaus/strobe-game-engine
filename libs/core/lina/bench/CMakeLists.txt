add_executable(strobe-core-lina-bench EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_googlebenchmark(strobe-core-lina-bench PUBLIC)
target_link_libraries(strobe-core-lina-bench
  PRIVATE strobe-core-lina
)

target_compile_definitions(strobe-core-lina-bench
  PRIVATE NDEBUG
)

target_compile_options(strobe-core-lina-bench
  PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-O3>
    $<$<CXX_COMPILER_ID:Clang>:-Wno-c2y-extensions>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

if(TARGET benchmark)
  target_compile_definitions(benchmark PRIVATE NDEBUG)
  target_compile_options(benchmark
    PRIVATE
      $<$<CXX_COMPILER_ID:Clang,GNU>:-O3>
      $<$<CXX_COMPILER_ID:Clang>:-Wno-c2y-extensions>
      $<$<CXX_COMPILER_ID:MSVC>:/O2>
  )
endif()
