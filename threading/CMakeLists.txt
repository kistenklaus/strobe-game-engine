
add_library(${PROJECT_NAME}_threading
        src/shared_memory_buffer/SharedMemoryBuffer.h
        src/shared_memory_buffer/ConstSharedMemoryBuffer.cpp
        src/shared_memory_buffer/ConstSharedMemoryBuffer.h
        src/shared_memory_buffer/DoubleBufferedSharedMemoryBuffer.cpp
        src/shared_memory_buffer/DoubleBufferedSharedMemoryBuffer.h
        src/shared_memory_buffer/SynchronizedSharedMemoryBuffer.cpp
        src/shared_memory_buffer/SynchronizedSharedMemoryBuffer.h
        src/shared_memory_buffer/ReadSharedMemoryBuffer.h src/shared_memory_buffer/WriteSharedMemoryBuffer.h src/shared_memory_buffer/SharedMemoryBuffer.h src/shared_memory_buffer/ConstSharedMemoryBuffer.cpp src/shared_memory_buffer/ConstSharedMemoryBuffer.h)

target_include_directories(${PROJECT_NAME}_threading
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
