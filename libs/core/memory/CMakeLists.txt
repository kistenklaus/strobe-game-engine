set(STROBE_CORE_MEMORY_LIB ${STROBE_CORE_LIB}-memory CACHE INTERNAL "")


add_library(${STROBE_CORE_MEMORY_LIB} 
  STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/PageAllocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/pages.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/static_test.cpp
)

# Use PUBLIC here to propagate the include directories
target_include_directories(${STROBE_CORE_MEMORY_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_precompile_headers(${STROBE_CORE_MEMORY_LIB}
#   PUBLIC
#   ${CMAKE_CURRENT_SOURCE_DIR}/include/strobe/memory.hpp
# )

add_subdirectory(test)
