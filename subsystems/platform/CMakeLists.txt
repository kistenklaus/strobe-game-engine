include_guard(GLOBAL)

set(STROBE_PLATFORM ${PROJECT_NAME}-platform CACHE INTERNAL "")

add_library(${STROBE_PLATFORM} EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/platform/platform.cpp
)

target_include_directories(${STROBE_PLATFORM} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_PLATFORM} PUBLIC ${STROBE_CORE_LIB})
