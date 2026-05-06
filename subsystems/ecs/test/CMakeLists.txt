set(STROBE_ECS_TESTS ${STROBE_ECS_SUBSYS}-test)

enable_testing()

add_executable(${STROBE_ECS_TESTS} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/worker_pool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/job_scheduler.cpp
)

target_link_googletest(${STROBE_ECS_TESTS} PUBLIC)
target_link_libraries(${STROBE_ECS_TESTS} PRIVATE ${STROBE_ECS_SUBSYS})
target_include_directories(${STROBE_ECS_TESTS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)


strobe_discover_tests(${STROBE_ECS_TESTS})

