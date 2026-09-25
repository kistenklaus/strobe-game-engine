add_library(strobe-core-events INTERFACE)

target_include_directories(strobe-core-events
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-core-events
  INTERFACE ${STROBE_CORE_MEMORY_LIB}
  INTERFACE ${STROBE_CORE_CONTAINER_LIB}
)

add_subdirectory(test)
