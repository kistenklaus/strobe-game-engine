

add_library(strobe-core-container INTERFACE)

target_include_directories(strobe-core-container
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src 
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(strobe-core-container 
  INTERFACE ${STROBE_CORE_MEMORY_LIB}
)

add_subdirectory(test)
add_subdirectory(bench)
