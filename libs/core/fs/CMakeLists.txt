set(STROBE_CORE_FS_LIB ${STROBE_CORE_LIB}-fs CACHE INTERNAL "")


if (WIN32) 
  set(STROBE_CORE_FS_LIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/cp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/exists.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/mkdir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/mv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/rm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/stat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/utility.cpp
  )
else()
  set(STROBE_CORE_FS_LIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/cp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/exists.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/mkdir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/mv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/rm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/stat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/utility.cpp
  )
endif()

add_library(${STROBE_CORE_FS_LIB} STATIC EXCLUDE_FROM_ALL ${STROBE_CORE_FS_LIB_SRC})

target_include_directories(${STROBE_CORE_FS_LIB} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/src 
  ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(${STROBE_CORE_FS_LIB} PUBLIC ${STROBE_CORE_MEMORY_LIB})
target_link_libraries(${STROBE_CORE_FS_LIB} PUBLIC ${STROBE_CORE_CONTAINERS_LIB})
target_link_fmt(${STROBE_CORE_FS_LIB} PUBLIC)

add_subdirectory(test)
