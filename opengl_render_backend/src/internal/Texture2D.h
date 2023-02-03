#pragma once

#include <sstream>
#include "glad/glad.h"
#include <strobe/assets.h>
#include <memory>

class Texture2D {
public:
    enum Format {
        RGBA = GL_RGBA,
        RGB = GL_RGB,
        RG = GL_RG,
        RED = GL_RED,
        DEPTH_STENCIL = GL_DEPTH_STENCIL,
        DEPTH = GL_DEPTH_COMPONENT
    };

    explicit Texture2D(int width, int height, Format internalFormat, Format dataFormat, const char *pixels);

    explicit Texture2D(ImageAsset &image, Format internalFormat);

    ~Texture2D() = default;

    inline void bind() const {
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    static inline void unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    const Format m_internalFormat;
    const int m_width;
    const int m_height;
    unsigned int m_id;

};
