#include "Geometrie.h"

#include <iostream>
#include "Buffer.h"

Geometrie::Geometrie(std::vector<VertexAttribute> attributes, std::optional<IndexAttribute> indices)
        : m_attributes(std::move(attributes)),
          m_indices(std::move(indices)),
          m_vaoId(0),
          m_drawMode(0) {
    assert(!m_attributes.empty());
    glGenVertexArrays(1, &m_vaoId);
    bind();

    for (const auto &attrib: m_attributes) {

        GLint size;
        GLenum type;
        GLboolean normalized;

        switch (attrib.m_type) {
            case VertexAttribute::Vec4:
                size = 4;
                type = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            case VertexAttribute::Vec3:
                size = 3;
                type = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            case VertexAttribute::Vec2:
                size = 2;
                type = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            case VertexAttribute::Float:
                size = 1;
                type = GL_FLOAT;
                normalized = GL_FALSE;
                break;
            default:
                throw std::runtime_error("[Unimplemented Error] no handling defined for attribute type");
        }

        attrib.m_buffer->bind();
        glVertexAttribPointer(attrib.m_location,
                              size,
                              type,
                              normalized,
                              static_cast<GLsizei>(attrib.m_stride),
                              (void *) attrib.m_offset);
        glEnableVertexAttribArray(attrib.m_location);
    }

    if (m_indices.has_value()) {
        m_indices->m_buffer->bind();
        switch (m_indices->m_mode) {
            case IndexAttribute::TRIANGLES:
                m_drawMode = GL_TRIANGLES;
                break;
            default:
                throw std::runtime_error("[Unimplemented Error] no handling defined for index attribute mode");

        }
    }
    unbind();
}

Geometrie::~Geometrie() {
    glDeleteVertexArrays(1, &m_vaoId);
}

void Geometrie::bind() const {
    glBindVertexArray(m_vaoId);
}

void Geometrie::unbind() {
    glBindVertexArray(0);
}

void Geometrie::drawElements(bool bindBefore, bool unbindAfter) {
    assert(m_indices.has_value());
    if (bindBefore)bind();
    glDrawElements(m_drawMode,
                   static_cast<GLsizei>(m_indices->getDrawCount()),
                   GL_UNSIGNED_INT,
                   nullptr);
    if (unbindAfter)unbind();
}
