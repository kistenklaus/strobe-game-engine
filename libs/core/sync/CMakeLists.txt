add_library(strobe-core-sync INTERFACE)

target_include_directories(strobe-core-sync
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-core-sync
  INTERFACE strobe-core-memory
  INTERFACE strobe-core-containers
  INTERFACE strobe::tracy
)

target_link_fmt(strobe-core-sync INTERFACE)

add_library(strobe::core-sync ALIAS strobe-core-sync)

add_subdirectory(test)
