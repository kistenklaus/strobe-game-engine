#pragma once

#include "../resource_id.h"
#include <cassert>
#include <functional>

namespace strobe {

    class ShaderProgram {
    public:
        explicit ShaderProgram(unsigned int *refCount,
                               std::function<void(internal::resource_id id)> destroyCallback,
                               internal::resource_id resourceId)
                : m_refCount(refCount),
                  m_destroyCallback(std::move(destroyCallback)),
                  m_id(resourceId) {
            assert(m_id.isValid());
            assert(m_refCount != nullptr);
            assert(m_destroyCallback != nullptr);
        }

        ShaderProgram(const ShaderProgram &other)
                : m_refCount(other.m_refCount),
                  m_destroyCallback(other.m_destroyCallback),
                  m_id(other.m_id) {
            (*m_refCount)++;
        }

        ShaderProgram &operator=(ShaderProgram &other) { //NOLINT
            if (m_refCount == other.m_refCount)return *this;
            m_refCount = other.m_refCount;
            m_destroyCallback = other.m_destroyCallback;
            m_id = other.m_id;
            (*m_refCount)++;
            return *this;
        }

        ShaderProgram(ShaderProgram &&other) noexcept
                : m_refCount(other.m_refCount),
                  m_destroyCallback(std::move(other.m_destroyCallback)),
                  m_id(other.m_id) {
            other.m_refCount = nullptr;
            other.m_destroyCallback = nullptr;
            other.m_id = internal::null_id;
        }

        ShaderProgram& operator=(ShaderProgram&& other) noexcept {
            m_refCount = other.m_refCount;
            m_destroyCallback = std::move(other.m_destroyCallback);
            m_id = other.m_id;
            other.m_refCount = nullptr;
            other.m_destroyCallback = nullptr;
            other.m_id = internal::null_id;
            return *this;
        }

        ~ShaderProgram() {
            (*m_refCount)--;
            if (*m_refCount < 0) {
                m_destroyCallback(m_id);
            }
            m_refCount = nullptr;
            m_id = internal::null_id;
        }

        [[nodiscard]] internal::resource_id id() const {
            return m_id;
        }

    private:
        unsigned int *m_refCount;
        std::function<void(internal::resource_id)> m_destroyCallback;
        internal::resource_id m_id;
    };

}