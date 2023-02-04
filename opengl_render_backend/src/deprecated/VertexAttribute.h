#pragma once

#include <memory>
#include <utility>

class Buffer;

class VertexAttribute {
    friend class Geometrie;

public:
    enum AttributeType {
        Vec4,
        Vec3,
        Vec2,
        Float,
    };

    explicit VertexAttribute(std::shared_ptr<Buffer> buffer,
                             unsigned int location,
                             AttributeType attributeType,
                             unsigned long int byteOffset,
                             int byteStride);

    ~VertexAttribute() = default;


private:
    const unsigned int m_location;
    const std::shared_ptr<Buffer> m_buffer;
    AttributeType m_type;
    unsigned int m_stride;
    unsigned long int m_offset;
};
