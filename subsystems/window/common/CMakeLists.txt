
set(STROBE_WINDOW_COMMON ${STROBE_WINDOW_LIB}-core CACHE INTERNAL "Strobe window core library target")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(${STROBE_WINDOW_COMMON} STATIC ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)


target_include_directories(${STROBE_WINDOW_COMMON} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${STROBE_WINDOW_COMMON} PUBLIC ${STROBE_CORE_LIB})
