set(SANDBOX_NAME ${STROBE_INPUT_SUBSYS}-sandbox)

add_executable(${SANDBOX_NAME} main.cpp)

target_link_libraries(${SANDBOX_NAME} PUBLIC ${STROBE_INPUT_SUBSYS})

