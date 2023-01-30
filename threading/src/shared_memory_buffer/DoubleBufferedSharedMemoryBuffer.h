#pragma once

#include "SharedMemoryBuffer.h"
#include <semaphore>

template<typename T, unsigned int Size>
class DoubleBufferedSharedMemoryBuffer : public SharedMemoryBuffer<T> {
private:

    class ReadRef : public SharedMemoryBuffer<T>::ReadRef {
    public:

        explicit ReadRef(DoubleBufferedSharedMemoryBuffer<T, Size> *ptr)
                : m_ptr(ptr) {}

        void acquire() override {
            m_ptr->m_writeSemaphore.acquire();
        }

        void release() override {
            m_ptr->m_readSemaphore.release();
        }

        const T *data() {
            return m_ptr->m_readPtr;
        }

        unsigned int size() {
            return m_readSize;
        }

    private:
        DoubleBufferedSharedMemoryBuffer<T, Size> *m_ptr;

    };

    class WriteRef : public SharedMemoryBuffer<T>::WriteRef {
    public:
        explicit WriteRef(DoubleBufferedSharedMemoryBuffer<T, Size> *ptr)
                : m_ptr(m_ptr) {

        }

        void acquire() {
            m_ptr->m_readSemaphore.acquire();
        }

        void release() {
            T *const tmp = m_ptr->m_readPtr;
            m_ptr->m_readPtr = m_ptr->m_writePtr;
            m_ptr->m_writePtr = tmp;
            m_readSize = m_writeSize;
            // swap buffers.
            m_ptr->m_writeSemaphore.release();
        }

        T *data() { return m_writePtr; }

        void setSize(unsigned int size) { m_writeSize = size; }

        DoubleBufferedSharedMemoryBuffer<T, Size> *m_ptr = nullptr;
    };

    explicit DoubleBufferedSharedMemoryBuffer()
            : m_readPtr(m_data),
              m_writePtr(m_data + Size),
              m_writeSize(0),
              m_readSize(0) {}

    ReadRef readRef() { return ReadRef(this); }

    WriteRef writeRef() { return WriteRef(this); }

private:

    T m_data[2 * Size];
    T *m_readPtr = nullptr;
    T *m_writePtr = nullptr;
    unsigned int m_writeSize;
    unsigned int m_readSize;

    std::binary_semaphore m_readSemaphore = std::binary_semaphore(1);
    std::binary_semaphore m_writeSemaphore = std::binary_semaphore(0);
};


