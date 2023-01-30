#pragma once

#include "SharedMemoryBuffer.h"
#include <shared_mutex>

template<typename T, unsigned int Size>
class SynchronizedSharedMemoryBuffer : public SharedMemoryBuffer<T> {
private:
    class ReadRef {
    public:

        explicit ReadRef(SynchronizedSharedMemoryBuffer<T, Size> ptr)
                : m_ptr(ptr) {}

        void acquire() {
            m_ptr->m_mutex.lock_shared();
        }

        void release() {
            m_ptr->m_mutex.unlock_shared();
        }

        const T *data() { return m_ptr->m_data; }

        unsigned int size() { return m_ptr->m_size; }

    private:
        SynchronizedSharedMemoryBuffer<T, Size> *m_ptr;
    };

    class WriteRef {
    public:

        explicit WriteRef(SynchronizedSharedMemoryBuffer<T, Size> *ptr)
                : m_ptr(ptr) {}

        void acquire() {
            m_ptr->m_mutex.lock();
        }

        void release() {
            m_ptr->m_mutex.unlock();
        }

        T *data() { return m_ptr->m_data; }

        void setSize(unsigned int size) { m_ptr->m_size = size; }

    private:
        SynchronizedSharedMemoryBuffer<T, Size> *m_ptr;
    };

public:

    explicit SynchronizedSharedMemoryBuffer(int size = 0)
        : m_size(size){ }

    virtual ReadRef readRef() {return ReadRef(this);}

    virtual WriteRef writeRef() {return WriteRef(this);}

private:
    T m_data[Size];
    std::shared_mutex m_mutex;
    unsigned int m_size;
};
