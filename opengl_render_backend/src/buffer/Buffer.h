#pragma once

#include "../resource_id.h"
#include <memory>
#include <functional>
#include <strobe/shared_memory_buffer.h>
#include <cassert>


namespace strobe {

    class Buffer {

    public:
        enum class Usage { //don't fuck with the assigned values !!!
            DYNAMIC_DRAW=0,
            STATIC_DRAW=1
        };
        enum class Type{ //don't fuck with the assigned values !!!
            VERTEX_BUFFER=0,
            ELEMENT_BUFFER=1,
            UNIFORM_BUFFER=2
        };

    public:
        using requestUpdate_signature = void(internal::resource_id, unsigned int, unsigned int);

        // This should actually be private.
        explicit Buffer(unsigned int* refCount,
                        std::function<void(internal::resource_id)> destroyCallback,
                        std::function<void(internal::resource_id, unsigned int, unsigned int)> requestUpdate,
                        std::shared_ptr<ReadSharedMemoryBuffer<char>> memory,
                        Usage usage,
                        Type type,
                        internal::resource_id id)
                : m_refCount(refCount),
                  m_destroy(std::move(destroyCallback)),
                  m_requestUpdate(std::move(requestUpdate)),
                  m_memory(std::move(memory)),
                  m_usage(usage),
                  m_type(type),
                  m_id(id) {
            assert(m_id.isValid());
        }

        Buffer(const Buffer &other) :
                m_refCount(other.m_refCount),
                m_destroy(other.m_destroy),
                m_requestUpdate(other.m_requestUpdate),
                m_memory(other.m_memory),
                m_usage(other.m_usage),
                m_type(other.m_type),
                m_id(other.m_id) {
            (*m_refCount)++;
        }

        Buffer &operator=(const Buffer &other) { // NOLINT(bugprone-unhandled-self-assignment)
            if(m_refCount == other.m_refCount)return *this;
            m_refCount = other.m_refCount;
            m_destroy = other.m_destroy;
            m_requestUpdate = other.m_requestUpdate;
            m_memory = other.m_memory;
            m_usage = other.m_usage;
            m_type = other.m_type;
            m_id = other.m_id;
            if (m_refCount != nullptr) {
                (*m_refCount)++;
            }
            return *this;
        }

        Buffer(Buffer &&other) noexcept:
                m_refCount(other.m_refCount),
                m_destroy(std::move(other.m_destroy)),
                m_requestUpdate(std::move(other.m_requestUpdate)),
                m_memory(std::move(other.m_memory)),
                m_usage(other.m_usage),
                m_type(other.m_type),
                m_id(other.m_id) {
            other.m_refCount = nullptr;
            other.m_memory = nullptr;
            other.m_destroy = nullptr;
            other.m_requestUpdate = nullptr;
            other.m_id = internal::null_id;
        }

        Buffer &operator=(Buffer &&other)  noexcept {
            m_refCount = other.m_refCount;
            m_destroy = other.m_destroy;
            m_requestUpdate = other.m_requestUpdate;
            m_memory = other.m_memory;
            m_usage = other.m_usage;
            m_type = other.m_type;
            m_id = other.m_id;

            other.m_refCount = nullptr;
            other.m_destroy = nullptr;
            other.m_requestUpdate = nullptr;
            other.m_memory = nullptr;
            other.m_id = internal::null_id;
            return *this;
        }

        ~Buffer() {
            (*m_refCount)--;
            if (*m_refCount == 0) {
                m_destroy(m_id);
            }
            m_id = internal::null_id;
        }

        [[nodiscard]] Usage usage() const {
            return m_usage;
        }

        [[nodiscard]] std::shared_ptr<ReadSharedMemoryBuffer<char>> getMemory() const {
            return m_memory;
        }

        void flush() {
            m_requestUpdate(m_id, 0u, m_memory->capacity());
        }

        void flushRange(unsigned int startByte, unsigned int endByte) {
            m_requestUpdate(m_id, startByte, endByte);
        }

        [[nodiscard]] bool operator==(const Buffer &other) const {
            return m_memory.get() == other.m_memory.get()
                   && m_usage == other.m_usage;
        }

        [[nodiscard]] internal::resource_id resourceId() const {
            assert(m_id != internal::null_id);
            return m_id;
        }

    private:
        unsigned int *m_refCount;
        std::function<void(internal::resource_id)> m_destroy;
        std::function<void(internal::resource_id, unsigned int, unsigned int)> m_requestUpdate;
        std::shared_ptr<ReadSharedMemoryBuffer<char>> m_memory;
        Usage m_usage;
        Type m_type;
        internal::resource_id m_id;
    };
}