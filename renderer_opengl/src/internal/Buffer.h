#pragma once

#include "glad/glad.h"
#include <cassert>



class Buffer {
public:
    enum Type {
        VertexBuffer = GL_ARRAY_BUFFER,
        ElementBuffer = GL_ELEMENT_ARRAY_BUFFER,
        UniformBuffer = GL_UNIFORM_BUFFER,
        INVALID_TYPE = 0,
    };

    enum Usage {
        STATIC_DRAW = GL_STATIC_DRAW,
        STATIC_READ = GL_STATIC_READ,
        STATIC_COPY = GL_STATIC_COPY,
        STATIC = STATIC_DRAW,
        DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
        DYNAMIC_READ = GL_DYNAMIC_READ,
        DYNAMIC_COPY = GL_DYNAMIC_COPY,
        DYNAMIC = DYNAMIC_DRAW,
        PREVIOUS_USAGE,
        INVALID_USAGE,
    };

    enum Access {
        READ_WRITE = GL_READ_WRITE,
        READ_ONLY = GL_READ_ONLY,
        WRITE_ONLY = GL_WRITE_ONLY,
        INVALID_ACCESS = 0,

    };

    explicit Buffer(Type type,
                    Usage usage,
                    unsigned int byteSize,
                    const void *data = nullptr);

    Buffer(Buffer &&other) noexcept;

    Buffer &operator=(Buffer &&other) noexcept;

    Buffer(Buffer &) = delete;

    Buffer &operator=(Buffer &) = delete;

    void bind() const;

    void unbind();

    static void unbind(Type type);

    void setSubData(unsigned int offset,
                    const void *data,
                    unsigned int byteSize,
                    bool bindBefore = false,
                    bool unbindAfter = false);

    void setData(const void *data,
                 unsigned int byteSize,
                 Usage usage = PREVIOUS_USAGE,
                 bool bindBefore = false,
                 bool unbindAfter = false);

    [[nodiscard]] void *mapData(Access access,
                                bool bindBefore = false,
                                bool unbindAfter = false);

    [[nodiscard]] void *mapSubData(Access access,
                                   unsigned int byteOffset,
                                   unsigned int byteSize,
                                   bool bindBefore = false,
                                   bool unbindAfter = false,
                                   bool invalidateRange = false,
                                   bool persistent = false);

    void unmap(bool bindBefore, bool unbindAfter);

    [[nodiscard]] unsigned int name() const;

private:
    Type m_type;
    Usage m_usage;
    unsigned int m_size;
    unsigned int m_id;

};
