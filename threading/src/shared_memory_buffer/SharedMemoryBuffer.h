#pragma once
#include "ReadSharedMemoryBuffer.h"
#include "WriteSharedMemoryBuffer.h"

namespace strobe{

    template <typename T>
    class SharedMemoryBuffer
            : public ReadSharedMemoryBuffer<T>, WriteSharedMemoryBuffer<T>{ };
}