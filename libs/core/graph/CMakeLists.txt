add_library(strobe-core-graph INTERFACE)

target_include_directories(strobe-core-graph
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-core-graph
  INTERFACE strobe-core-memory
  INTERFACE strobe-core-containers
)

add_library(strobe::core-graph ALIAS strobe-core-graph)

add_subdirectory(test)
