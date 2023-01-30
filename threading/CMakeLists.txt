
add_library(strobe_threading_lib
        src/shared_memory_buffer/SharedMemoryBuffer.h
        src/shared_memory_buffer/ConstSharedMemoryBuffer.cpp
        src/shared_memory_buffer/ConstSharedMemoryBuffer.h
        src/shared_memory_buffer/DoubleBufferedSharedMemoryBuffer.cpp
        src/shared_memory_buffer/DoubleBufferedSharedMemoryBuffer.h
        src/shared_memory_buffer/SynchronizedSharedMemoryBuffer.cpp
        src/shared_memory_buffer/SynchronizedSharedMemoryBuffer.h
        )

target_include_directories(strobe_threading_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
