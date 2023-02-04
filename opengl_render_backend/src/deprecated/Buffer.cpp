#include "Buffer.h"

Buffer::Buffer(Buffer::Type type,
               Buffer::Usage usage,
               unsigned int size,
               const void *data)
        : m_type(type),
          m_usage(usage),
          m_size(size),
          m_id(0) {
    assert(m_usage != PREVIOUS_USAGE);
    assert(m_usage != INVALID_USAGE);
    assert(m_type != INVALID_TYPE);
    glGenBuffers(1, &m_id);
    assert(m_id != 0);
    bind();

    glBufferData(m_type,
                 static_cast<GLsizeiptr>(m_size),
                 data,
                 m_usage);

    unbind();
}

Buffer::Buffer(Buffer &&other) noexcept
        : m_type(other.m_type),
          m_usage(other.m_usage),
          m_size(other.m_size),
          m_id(other.m_id) {
    other.m_id = 0;
    other.m_usage = INVALID_USAGE;
    m_size = 0;
    m_type = INVALID_TYPE;
}


Buffer &Buffer::operator=(Buffer &&other) noexcept {
    m_type = other.m_type;
    m_usage = other.m_usage;
    m_size = other.m_size;
    m_id = other.m_id;
    other.m_type = INVALID_TYPE;
    other.m_usage = INVALID_USAGE;
    other.m_size = 0;
    other.m_id = 0;
    return *this;
}

void Buffer::setSubData(unsigned int offset,
                        const void *data,
                        unsigned int size,
                        bool bindBefore,
                        bool unbindAfter) {
    assert(m_type != INVALID_TYPE);
    if (bindBefore)bind();
    glBufferSubData(m_type, offset, size, data);
    if (unbindAfter)unbind();
}

void Buffer::setData(const void *data, unsigned int size, Buffer::Usage usage, bool bindBefore, bool unbindAfter) {
    assert(usage != INVALID_USAGE);
    assert(m_type != INVALID_TYPE);
    if (bindBefore)bind();
    if (usage != PREVIOUS_USAGE) m_usage = usage;
    m_size = size;
    glBufferData(m_type,
                 static_cast<GLsizeiptr>(m_size),
                 data,
                 m_usage);
    if (unbindAfter)unbind();
}

void *Buffer::mapData(Buffer::Access access, bool bindBefore, bool unbindAfter) {
    assert(m_type != INVALID_TYPE);
    assert(access != INVALID_ACCESS);
    if (bindBefore)bind();
    void *mappedMemory = glMapBuffer(m_type, access);
    if (unbindAfter)unbind();
    return mappedMemory;
}

void *Buffer::mapSubData(Buffer::Access access,
                         unsigned int offset,
                         unsigned int size,
                         bool bindBefore,
                         bool unbindAfter,
                         bool invalidateRange, bool persistent) {
    assert(access != INVALID_ACCESS);
    assert(m_type != INVALID_TYPE);
    GLbitfield accessFlags = 0;
    if (access == READ_ONLY || access == READ_WRITE) {
        accessFlags |= GL_MAP_READ_BIT;
        assert(!invalidateRange);
    }
    if (access == WRITE_ONLY || access == READ_WRITE) {
        accessFlags |= GL_MAP_WRITE_BIT;
    }
    if (invalidateRange) {
        accessFlags |= GL_MAP_INVALIDATE_RANGE_BIT;
        assert((accessFlags & GL_MAP_READ_BIT) == 0);
    }
    if (persistent) {
        accessFlags |= GL_MAP_PERSISTENT_BIT;
    }
    if (bindBefore)bind();
    void *mappedMemory = glMapBufferRange(m_type, offset, size, accessFlags);
    if (unbindAfter)unbind();
    return mappedMemory;
}

void Buffer::unmap(bool bindBefore, bool unbindAfter) {
    if(bindBefore)bind();
    glUnmapBuffer(m_type);
    if(unbindAfter)unbind();
}

void Buffer::bind() const {
    glBindBuffer(m_type, m_id);
}

void Buffer::unbind() {
    glBindBuffer(m_type, 0);
}

void Buffer::unbind(Buffer::Type type) {
    glBindBuffer(type, 0);
}

unsigned int Buffer::name() const {
    return m_id;
}

