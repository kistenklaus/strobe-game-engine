

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/stb_image")

add_library(${PROJECT_NAME}_assets
        src/assets/ImageAsset.cpp
        src/assets/ImageAsset.h
        src/assets/TextFileAsset.cpp
        src/assets/TextFileAsset.h
        include/strobe/assets.h ../threading/include/strobe/shared_memory_buffer.h)

target_link_libraries(${PROJECT_NAME}_assets PRIVATE stb_image)

target_include_directories(${PROJECT_NAME}_assets PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
