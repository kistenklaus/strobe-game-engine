include_guard(GLOBAL)

add_library(strobe-flow
  src/strobe/flow/compile_context.cpp
  src/strobe/flow/compiler_plugins.cpp
  src/strobe/flow/flow.cpp
  src/strobe/flow/op_stream.cpp
  src/strobe/flow/op_source.cpp
  src/strobe/flow/program.cpp
)

target_include_directories(strobe-flow
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(strobe-flow PUBLIC strobe-core)

target_compile_features(strobe-flow PUBLIC cxx_std_23)

add_library(strobe::flow ALIAS strobe-flow)

add_subdirectory(sandbox)

add_subdirectory(test)
