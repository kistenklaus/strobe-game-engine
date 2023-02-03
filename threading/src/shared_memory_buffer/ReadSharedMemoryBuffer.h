#pragma once


namespace strobe{

    template<typename T>
    class ReadSharedMemoryBuffer{
    public:
        virtual ~ReadSharedMemoryBuffer() = default;
        virtual void acquireNewRead() = 0;
        virtual void releaseRead() = 0;
        virtual const T* data() const = 0;
        [[nodiscard]] virtual unsigned int size() const = 0;
        [[nodiscard]] virtual unsigned int capacity() const = 0;
    };

}