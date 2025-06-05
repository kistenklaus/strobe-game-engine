set(STROBE_CORE_LIB ${PROJECT_NAME}-core CACHE INTERNAL "")

add_subdirectory(memory)
add_subdirectory(lina)
add_subdirectory(containers)
add_subdirectory(sync)
add_subdirectory(events)
add_subdirectory(fs)

add_library(${STROBE_CORE_LIB} INTERFACE)

target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_MEMORY_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_LINA_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_SYNC_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_CONTAINERS_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_EVENTS_LIB})
target_link_libraries(${STROBE_CORE_LIB} INTERFACE ${STROBE_CORE_FS_LIB})

