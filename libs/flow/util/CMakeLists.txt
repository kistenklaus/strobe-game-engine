add_library(strobe-flow-util INTERFACE)

target_include_directories(strobe-flow-util
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-flow-util
  INTERFACE strobe::flow
)

target_compile_features(strobe-flow-util INTERFACE cxx_std_23)

add_library(strobe::flow-util ALIAS strobe-flow-util)

add_subdirectory(sandbox)
