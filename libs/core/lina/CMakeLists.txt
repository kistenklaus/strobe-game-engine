
add_library(strobe-core-lina INTERFACE)

target_include_directories(strobe-core-lina
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-core-lina
  INTERFACE strobe-core-containers
)

add_subdirectory(test)
add_subdirectory(bench)
