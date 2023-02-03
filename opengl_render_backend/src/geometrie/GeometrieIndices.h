#pragma once

#include "../buffer/Buffer.h"

namespace strobe {

    enum class GeometrieIndexType {
        U32,
    };

    class GeometrieIndices {

    public:
        explicit GeometrieIndices(std::shared_ptr<Buffer> buffer,
                                  GeometrieIndexType type)
                : m_buffer(std::move(buffer)),
                  m_type(type) {
        }

        explicit GeometrieIndices(const Buffer& buffer,
                                  GeometrieIndexType type)
                : m_buffer(std::make_shared<Buffer>(buffer)),
                  m_type(type) {
        }

        [[nodiscard]] std::shared_ptr<Buffer> buffer() const {
            return m_buffer;
        }

        [[nodiscard]] GeometrieIndexType type() const {
            return m_type;
        }

        bool operator==(const GeometrieIndices &other) const{
            return m_buffer == other.m_buffer
                   && m_type == other.m_type;
        }

    private:
        std::shared_ptr<Buffer> m_buffer;
        GeometrieIndexType m_type;
    };

}