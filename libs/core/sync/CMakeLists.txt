set(STROBE_CORE_SYNC_LIB ${STROBE_CORE_LIB}-sync CACHE INTERNAL "")


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_CORE_SYNC_LIB} INTERFACE)

target_include_directories(${STROBE_CORE_SYNC_LIB} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${STROBE_CORE_SYNC_LIB} INTERFACE ${STROBE_CORE_MEMORY_LIB})

if (STROBE_BUILD_TESTS)
  add_subdirectory(test)
endif()
