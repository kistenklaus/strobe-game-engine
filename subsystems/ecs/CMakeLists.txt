include_guard(GLOBAL)

set(STROBE_ECS_SUBSYS ${PROJECT_NAME}-ecs CACHE INTERNAL "")

add_library(${STROBE_ECS_SUBSYS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/ecs/ecs.cpp
)
target_include_directories(${STROBE_ECS_SUBSYS} 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_ECS_SUBSYS} PUBLIC ${STROBE_WINDOW_SUBSYS})

add_subdirectory(test)
add_subdirectory(bench)
