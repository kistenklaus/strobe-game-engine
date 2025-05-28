set(STROBE_CORE_EVENTS_LIB ${STROBE_CORE_LIB}-events CACHE INTERNAL "")


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_CORE_EVENTS_LIB} 
  STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

# Use PUBLIC here to propagate the include directories
target_include_directories(${STROBE_CORE_EVENTS_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${STROBE_CORE_EVENTS_LIB} PUBLIC ${STROBE_CORE_MEMORY_LIB})
target_link_libraries(${STROBE_CORE_EVENTS_LIB} PUBLIC ${STROBE_CORE_SYNC_LIB})
target_link_libraries(${STROBE_CORE_EVENTS_LIB} PUBLIC ${STROBE_CORE_CONTAINER_LIB})

# target_precompile_headers(${STROBE_CORE_EVENTS_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/strobe/events.hpp)

add_subdirectory(test)
