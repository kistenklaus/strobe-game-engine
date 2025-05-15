

set(STROBE_CORE_LIB ${PROJECT_NAME}-core-lib CACHE INTERNAL "")



add_subdirectory(memory)
add_subdirectory(lina)
add_subdirectory(containers)
add_subdirectory(sync)

add_library(${STROBE_CORE_LIB} INTERFACE)
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_MEMORY_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_LINA_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_SYNC_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_CONTAINERS_LIB})

