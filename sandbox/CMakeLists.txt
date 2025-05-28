
set(SANDBOX ${PROJECT_NAME}-sandbox)

add_executable(${SANDBOX}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(${SANDBOX} PUBLIC
  ${STROBE_WINDOW_SUBSYS})
    

