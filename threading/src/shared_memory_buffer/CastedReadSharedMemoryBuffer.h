#pragma once

#include "ReadSharedMemoryBuffer.h"
#include <memory>

namespace strobe {

    template<typename T, typename O>
    class CastedReadSharedMemoryBuffer : public ReadSharedMemoryBuffer<T> {
    public:
        explicit CastedReadSharedMemoryBuffer(std::shared_ptr<ReadSharedMemoryBuffer<O>> origin)
                : m_origin(std::move(origin)) {
            static_assert(sizeof(O) % sizeof(T) == 0);
        }

        void acquireNewRead() override {
            m_origin->acquireNewRead();
        }

        void releaseRead() override {
            m_origin->releaseRead();
        }

        const T *data() const override {
            return reinterpret_cast<const T *>(m_origin->data());
        }

        [[nodiscard]] unsigned int size() const override {
            return m_origin->size() * (sizeof(O) / sizeof(T));
        }

        [[nodiscard]] unsigned int capacity() const override {
            return m_origin->capacity() * (sizeof(O) / sizeof(T));
        }

    private:
        std::shared_ptr<ReadSharedMemoryBuffer<O>> m_origin;
    };

}