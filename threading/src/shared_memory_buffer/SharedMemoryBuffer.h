#pragma once


template<typename T>
class SharedMemoryBuffer {
public:

    class ReadRef {
    public:
        virtual ~ReadRef() = default;

        virtual void acquire() = 0;

        virtual void release() = 0;

        virtual const T *data() = 0;

        virtual unsigned int size() = 0;
    };

    class WriteRef {
    public:
        virtual ~WriteRef() = default;

        virtual void acquire() = 0;

        virtual void release() = 0;

        virtual T *data() = 0;

        virtual void setSize(unsigned int size) = 0;
    };

    virtual ~SharedMemoryBuffer() = default;

    SharedMemoryBuffer(SharedMemoryBuffer &) = delete;

    SharedMemoryBuffer(SharedMemoryBuffer &&) = delete;

    virtual SharedMemoryBuffer &operator=(SharedMemoryBuffer &) = delete;

    virtual SharedMemoryBuffer &operator=(SharedMemoryBuffer &&) = delete;

    virtual ReadRef readRef() = 0;

    virtual WriteRef writeRef() = 0;

protected:
    SharedMemoryBuffer() = default;
};
