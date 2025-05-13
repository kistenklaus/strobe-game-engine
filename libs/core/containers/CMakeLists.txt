set(STROBE_CORE_CONTAINERS_LIB ${STROBE_CORE_LIB}-containers CACHE INTERNAL "")


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_CORE_CONTAINERS_LIB} 
  STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

# Use PUBLIC here to propagate the include directories
target_include_directories(${STROBE_CORE_CONTAINERS_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${STROBE_CORE_CONTAINERS_LIB} PUBLIC ${STROBE_CORE_MEMORY_LIB})

# target_precompile_headers(${STROBE_CORE_CONTAINERS_LIB}
#   PUBLIC
#   ${CMAKE_CURRENT_SOURCE_DIR}/include/strobe/containers.hpp
# )

add_subdirectory(test)
