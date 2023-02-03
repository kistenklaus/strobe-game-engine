#pragma once

namespace strobe{
    template <typename T>
    class WriteSharedMemoryBuffer{
    public:
        virtual ~WriteSharedMemoryBuffer() = default;
        virtual void acquireNewWrite() = 0;
        virtual void releaseWrite() = 0;
        virtual T* data() = 0;
        virtual void setSize(unsigned int size) = 0;
        [[nodiscard]] virtual unsigned int capacity() const = 0;
    };

}