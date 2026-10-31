
add_library(strobe-core-color INTERFACE)

target_include_directories(strobe-core-color 
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-core-color 
  INTERFACE strobe-core-lina
)


add_subdirectory(test)
