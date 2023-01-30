#include "VertexAttribute.h"

#include <cassert>

VertexAttribute::VertexAttribute(std::shared_ptr<Buffer> buffer,
                                 unsigned int location,
                                 VertexAttribute::AttributeType attributeType,
                                 unsigned long offset, int stride)
        : m_location(location),
          m_buffer(std::move(buffer)),
          m_type(attributeType),
          m_stride(stride),
          m_offset(offset) { }
