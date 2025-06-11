set(STROBE_CORE_FS_LIB ${STROBE_CORE_LIB}-fs CACHE INTERNAL "")


add_library(${STROBE_CORE_FS_LIB} STATIC EXCLUDE_FROM_ALL
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/cp_linux.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/exists.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/mkdir.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/mv.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/rm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/stat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/core/fs/utility.cpp
)

target_include_directories(${STROBE_CORE_FS_LIB} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/src 
  ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(${STROBE_CORE_FS_LIB} PUBLIC ${STROBE_CORE_MEMORY_LIB})
target_link_libraries(${STROBE_CORE_FS_LIB} PUBLIC ${STROBE_CORE_CONTAINERS_LIB})
target_link_fmt(${STROBE_CORE_FS_LIB} PUBLIC)

add_subdirectory(test)
