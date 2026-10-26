add_executable(strobe-rdg-sandbox EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(strobe-rdg-sandbox 
  PRIVATE 
    strobe::rdg
    strobe-platform
)

