add_executable(strobe-flow-util-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(strobe-flow-util-sandbox PRIVATE strobe::flow-util)
