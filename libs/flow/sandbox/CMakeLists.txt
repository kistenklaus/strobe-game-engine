add_executable(strobe-flow-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(strobe-flow-sandbox PRIVATE strobe::flow)
