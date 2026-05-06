set(STROBE_ECS_BENCH ${STROBE_ECS_SUBSYS}-bench)

enable_testing()

add_executable(${STROBE_ECS_BENCH} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_googlebenchmark(${STROBE_ECS_BENCH} PUBLIC)
target_link_libraries(${STROBE_ECS_BENCH} PRIVATE ${STROBE_ECS_SUBSYS})
target_include_directories(${STROBE_ECS_BENCH}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/bench)


strobe_discover_tests(${STROBE_ECS_BENCH})

