set(STROBE_CORE_FS_TESTS ${STROBE_CORE_FS_LIB}-test)


enable_testing()

add_executable(${STROBE_CORE_FS_TESTS} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/Path.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_googletest(${STROBE_CORE_FS_TESTS} PRIVATE)

target_link_libraries(${STROBE_CORE_FS_TESTS} PRIVATE ${STROBE_CORE_FS_LIB})

strobe_discover_tests(${STROBE_CORE_FS_TESTS}) 
