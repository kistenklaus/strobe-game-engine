#pragma once

#include "SharedMemoryBuffer.h"
#include <stdexcept>
#include <cstring>

namespace strobe {

    template<typename T, unsigned int Size>
    class ConstSharedMemoryBuffer : public SharedMemoryBuffer<T> {
    public:
        explicit ConstSharedMemoryBuffer(T *initialValue, unsigned int size = Size)
                : m_size(size){
            std::memcpy(m_data, initialValue, m_size);
        }

        void acquireNewRead() override {}

        void releaseRead() override {}

        void acquireNewWrite() override {
            throw std::logic_error("Can't acquire write from ConstSharedMemoryBuffer");
        }

        void releaseWrite() override {
            throw std::logic_error("Can't releaseWrite from ConstSharedMemoryBuffer");
        }

        const T *data() const override { return m_data; }

        T *data() override {
            throw std::logic_error("Can't write to ConstSharedMemoryBuffer");
        }

        [[nodiscard]] unsigned int size() const override { return m_size; }

        void setSize(unsigned int size) override {
            m_size = size;
        }

        [[nodiscard]] unsigned int capacity() const override { return Size; }

    private:
        union {
            T m_data[Size];
            char m_bytes[Size * sizeof(T)];
        };
        unsigned int m_size;
    };

}