

if (WIN32) 
  set(STROBE_CORE_FS_LIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/cp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/exists.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/mkdir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/mv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/rm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/stat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/win/directory.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/unix/directory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/utility.cpp
  )
endif()

add_library(strobe-core-fs STATIC EXCLUDE_FROM_ALL 
  ${STROBE_CORE_FS_LIB_SRC}
)

target_include_directories(strobe-core-fs 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)


target_link_libraries(strobe-core-fs 
  PUBLIC ${STROBE_CORE_MEMORY_LIB}
  PUBLIC strobe-core-containers
)
target_link_fmt(strobe-core-fs PUBLIC)

add_subdirectory(test)
