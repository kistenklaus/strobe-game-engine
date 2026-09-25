
add_library(strobe-core-memory EXCLUDE_FROM_ALL
  STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/PageAllocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/pages.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/new_instrument.cpp
)

target_include_directories(strobe-core-memory 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-core-memory
  PUBLIC strobe::tracy
  PUBLIC strobe-core-type-traits
)

add_subdirectory(test)
