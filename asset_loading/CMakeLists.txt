

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/stb_image")

add_library(assets
        src/assets/ImageAsset.cpp
        src/assets/ImageAsset.h
        src/assets/TextFileAsset.cpp
        src/assets/TextFileAsset.h
        include/strobe/assets.h ../threading/include/strobe/shared_memory_buffer.h)

target_link_libraries(assets PRIVATE stb_image)

target_include_directories(assets PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
