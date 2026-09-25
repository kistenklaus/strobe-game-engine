enable_testing()

add_executable(strobe-core-fs-test EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Path.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exists.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mkdir.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mv.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/stat.cpp
)

target_link_googletest(strobe-core-fs-test PRIVATE)

target_link_libraries(strobe-core-fs-test 
  PRIVATE strobe-core-fs
)

strobe_discover_tests(strobe-core-fs-test) 
