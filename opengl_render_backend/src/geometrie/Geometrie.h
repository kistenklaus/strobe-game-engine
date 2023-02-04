#pragma once

#include <iostream>
#include <functional>
#include "GeometrieIndices.h"
#include "GeometrieAttribute.h"
#include "../resource_id.h"

namespace strobe {

    class Geometrie {
    public:
        explicit Geometrie(unsigned int *refCount,
                           std::function<void(internal::resource_id)> destroyCallback,
                           std::function<void(internal::resource_id)> requestUpdateCallback,
                           std::vector<GeometrieAttribute> attributes,
                           std::shared_ptr<GeometrieIndices> indices,
                           internal::resource_id resourceId)
                : m_refCount(refCount),
                  m_destroy(std::move(destroyCallback)),
                  m_requestUpdate(std::move(requestUpdateCallback)),
                  m_attributes(std::move(attributes)),
                  m_indices(std::move(indices)),
                  m_id(resourceId) {
            assert(m_id.isValid());
        }

        Geometrie(const Geometrie &other)
                : m_refCount(other.m_refCount),
                  m_destroy(other.m_destroy),
                  m_requestUpdate(other.m_requestUpdate),
                  m_attributes(other.m_attributes),
                  m_indices(other.m_indices),
                  m_id(other.m_id) {
            if (m_refCount) {
                (*m_refCount)++;
            }
        }

        Geometrie &operator=(const Geometrie &other) { // NOLINT(bugprone-unhandled-self-assignment)
            if (m_refCount == other.m_refCount) {
                return *this;
            }
            m_refCount = other.m_refCount;
            m_destroy = other.m_destroy;
            m_requestUpdate = other.m_requestUpdate;
            m_attributes = other.m_attributes;
            m_indices = other.m_indices;
            m_id = other.m_id;
            if (m_refCount) {
                (*m_refCount)++;
            }
            return *this;
        }

        Geometrie(Geometrie &&other) noexcept
                : m_refCount(other.m_refCount),
                  m_destroy(std::move(other.m_destroy)),
                  m_requestUpdate(std::move(other.m_requestUpdate)),
                  m_attributes(std::move(other.m_attributes)),
                  m_indices(std::move(other.m_indices)),
                  m_id(other.m_id){

            other.m_refCount = nullptr;
            other.m_destroy = nullptr;
            other.m_requestUpdate = nullptr;
            other.m_attributes.clear();
            other.m_indices = nullptr;
            other.m_id = internal::null_id;
        }

        Geometrie& operator=(Geometrie&& other) noexcept {
            if(m_refCount == other.m_refCount)return *this;
            m_refCount = other.m_refCount;
            m_destroy = std::move(other.m_destroy);
            m_requestUpdate = std::move(other.m_requestUpdate);
            m_attributes = std::move(other.m_attributes);
            m_indices = std::move(other.m_indices);
            m_id = other.m_id;
            other.m_refCount = nullptr;
            other.m_destroy = nullptr;
            other.m_requestUpdate = nullptr;
            //other.m_attributes.clear(); //don't thing this would actually be allowed.
            m_indices = nullptr;
            other.m_id  = internal::null_id;
            return *this;
        }

        ~Geometrie() {
            (*m_refCount)--;
            if (*m_refCount == 0) {
                m_destroy(m_id);
            }
            m_id = internal::null_id;
        }

        std::vector<GeometrieAttribute>& getAttributes() {
            return m_attributes;
        }

        std::shared_ptr<GeometrieIndices>& getIndices(){
            return m_indices;
        }
        [[nodiscard]] internal::resource_id id() const {
            assert(m_id.isValid());
            return m_id;
        }

    private:
        unsigned int *m_refCount;
        std::function<void(internal::resource_id)> m_destroy;
        std::function<void(internal::resource_id)> m_requestUpdate;
        std::vector<GeometrieAttribute> m_attributes;
        std::shared_ptr<GeometrieIndices> m_indices;
        internal::resource_id m_id;
    };

}
