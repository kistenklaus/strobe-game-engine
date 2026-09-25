

add_library(strobe-core-containers INTERFACE)

target_include_directories(strobe-core-containers
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(strobe-core-containers
  INTERFACE strobe-core-memory
)

add_subdirectory(test)
add_subdirectory(bench)
