

set(STROBE_WINDOW_COMMON ${STROBE_WINDOW_LIB}-core CACHE INTERNAL "Strobe window core library target")

add_library(${STROBE_WINDOW_COMMON} INTERFACE)

target_include_directories(${STROBE_WINDOW_COMMON} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${STROBE_WINDOW_COMMON} INTERFACE ${STROBE_CORE_LIB})
