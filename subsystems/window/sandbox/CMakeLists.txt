

set(SANDBOX_NAME ${STROBE_WINDOW_LIB}-sandbox)

add_executable(${SANDBOX_NAME} main.cpp)

target_link_libraries(${SANDBOX_NAME} PUBLIC ${STROBE_WINDOW_LIB})

