set(STROBE_CORE_MEMORY_LIB ${STROBE_CORE_LIB}-memory CACHE INTERNAL "")


add_library(${STROBE_CORE_MEMORY_LIB} EXCLUDE_FROM_ALL
  STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/PageAllocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/pages.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/memory/static_test.cpp
)

target_include_directories(${STROBE_CORE_MEMORY_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (STROBE_BUILD_TESTS)
  add_subdirectory(test)
endif()
