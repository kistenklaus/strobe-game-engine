#pragma once

#include "../buffer/Buffer.h"
#include <memory>

namespace strobe {

    class GeometrieAttribute {
    public:
        enum struct Type {
            Vec4_32f,
            Vec3_32f,
            Vec2_32f,
        };

        explicit GeometrieAttribute(unsigned int location,
                                    Type type,
                                    std::shared_ptr<Buffer> buffer,
                                    unsigned int stride,
                                    unsigned int offset)
                : m_location(location),
                  m_type(type),
                  m_buffer(std::move(buffer)),
                  m_byteStride(stride),
                  m_byteOffset(offset) {}

        explicit GeometrieAttribute(unsigned int location,
                                    Type type,
                                    const Buffer& buffer,
                                    unsigned int stride,
                                    unsigned int offset)
                : m_location(location),
                  m_type(type),
                  m_buffer(std::make_shared<Buffer>(buffer)),
                  m_byteStride(stride),
                  m_byteOffset(offset) {}

        [[nodiscard]] unsigned int location() const { return m_location; }

        [[nodiscard]] Type type() const { return m_type; }

        [[nodiscard]] std::shared_ptr<Buffer> buffer() const { return m_buffer; }

        [[nodiscard]] unsigned int byteStride() const { return m_byteStride; }

        [[nodiscard]] unsigned int byteOffset() const { return m_byteOffset; }

        bool operator==(const GeometrieAttribute &other) const {
            return
                    m_buffer == other.m_buffer
                    && m_location == other.m_location
                    && m_type == other.m_type
                    && m_byteStride == other.m_byteStride
                    && m_byteOffset == other.m_byteOffset;
        }

        bool operator<(const GeometrieAttribute &other) const {
            return m_location < other.m_location;
        }

        bool operator>(const GeometrieAttribute &other) const {
            return m_location > other.m_location;
        }

        bool operator<=(const GeometrieAttribute &other) const {
            return m_location <= other.m_location;
        }

        bool operator>=(const GeometrieAttribute &other) const {
            return m_location >= other.m_location;
        }

    private:
        unsigned int m_location;
        Type m_type;
        std::shared_ptr<Buffer> m_buffer;
        unsigned int m_byteStride;
        unsigned int m_byteOffset;
    };

}