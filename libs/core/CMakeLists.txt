set(STROBE_CORE_LIB ${PROJECT_NAME}-core CACHE INTERNAL "")

add_subdirectory(memory)
add_subdirectory(lina)
add_subdirectory(containers)
add_subdirectory(events)
add_subdirectory(fs)
add_subdirectory(type_traits)

add_library(strobe-core INTERFACE)

target_link_libraries(strobe-core 
  INTERFACE strobe-core-memory
  INTERFACE ${STROBE_CORE_LINA_LIB}
  INTERFACE strobe-core-containers
  INTERFACE strobe-core-events
  INTERFACE strobe-core-fs
  INTERFACE ${STROBE_CORE_TYPE_TRAITS_LIB}
)

target_include_directories(strobe-core
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(strobe-core
  INTERFACE <strobe/core.hpp>
)
