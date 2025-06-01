set(STROBE_CORE_LINA_LIB ${STROBE_CORE_LIB}-lina CACHE INTERNAL "")


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_CORE_LINA_LIB} 
  STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

# Use PUBLIC here to propagate the include directories
target_include_directories(${STROBE_CORE_LINA_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_precompile_headers(${STROBE_CORE_LINA_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/strobe/lina.hpp)
