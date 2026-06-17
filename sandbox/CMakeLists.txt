
set(SANDBOX ${PROJECT_NAME}-sandbox)

add_executable(${SANDBOX} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(${SANDBOX}
  PUBLIC ${STROBE_WINDOW_SUBSYS}
  PUBLIC ${STROBE_ECS_SUBSYS}
)
    

