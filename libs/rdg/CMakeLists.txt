include_guard(GLOBAL)

add_library(strobe-rdg
  src/strobe/rdg/detail/compiler/buffer_barriers.cpp
  src/strobe/rdg/detail/compiler/device_order.cpp
  src/strobe/rdg/detail/compiler/globals.cpp
  src/strobe/rdg/detail/compiler/image_barriers.cpp
  src/strobe/rdg/detail/compiler/materialize.cpp
  src/strobe/rdg/detail/compiler/materializer.cpp
  src/strobe/rdg/detail/compiler/resource_accesses.cpp
  src/strobe/rdg/detail/compiler/resource_states.cpp
  src/strobe/rdg/detail/compiler/submissions.cpp
  src/strobe/rdg/detail/compiler/swapchain.cpp
  src/strobe/rdg/detail/compiler/util.cpp
  src/strobe/rdg/plugins.cpp
  src/strobe/rdg/rdg.cpp
)

target_include_directories(strobe-rdg
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-rdg
  PUBLIC strobe::flow strobe::rhi strobe::flow-util
)

target_compile_features(strobe-rdg PUBLIC cxx_std_23)

add_library(strobe::rdg ALIAS strobe-rdg)

add_subdirectory(sandbox)

add_subdirectory(test)
