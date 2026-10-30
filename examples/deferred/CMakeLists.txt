


add_executable(example-deferred EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_include_directories(example-deferred 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(example-deferred
  PRIVATE strobe-platform
  PRIVATE strobe::rhi
)

target_link_fmt(example-deferred PRIVATE)
