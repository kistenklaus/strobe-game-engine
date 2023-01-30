#pragma once

#include "SharedMemoryBuffer.h"
#include <cstring>
#include <stdexcept>

template<typename T, unsigned int Size>
class ConstSharedMemoryBuffer : public SharedMemoryBuffer<T>{
private:

    class ReadRef {
    public:
        explicit ReadRef(ConstSharedMemoryBuffer<T, Size> ptr)
            : m_ptr(ptr) {}

        void acquire() { }

        void release() { }

        const T *data() { return m_ptr->m_data;}

        unsigned int size() { return Size; }
    private:
        ConstSharedMemoryBuffer<T, Size> m_ptr;
    };

public:

    explicit ConstSharedMemoryBuffer(T* bufferValue){
        std::memcpy(m_data, bufferValue, Size * sizeof(T));
    }

    virtual ReadRef readRef() { return ReadRef(this); }

    typename SharedMemoryBuffer<T>::WriteRef writeRef() {
        throw std::logic_error("Can't create a read reference to a ConstSharedMemoryBuffer");
    }

private:
    T m_data[Size];
};
