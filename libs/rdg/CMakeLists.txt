include_guard(GLOBAL)

add_library(strobe-rdg
  src/strobe/rdg/rdg.cpp
)

target_include_directories(strobe-rdg
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(strobe-rdg
  PUBLIC strobe::flow strobe::rhi
)

target_compile_features(strobe-rdg PUBLIC cxx_std_23)

add_library(strobe::rdg ALIAS strobe-rdg)

add_subdirectory(test)
