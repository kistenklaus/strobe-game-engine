include_guard(GLOBAL)

set(STROBE_ASSETS_SUBSYS ${PROJECT_NAME}-assets CACHE INTERNAL "")

add_library(${STROBE_ASSETS_SUBSYS} 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strobe/assets/AssetManager.cpp
)
target_include_directories(${STROBE_ASSETS_SUBSYS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${STROBE_ASSETS_SUBSYS} PUBLIC ${STROBE_CORE_LIB})

add_subdirectory(preprocessor)
add_subdirectory(database)

