#include "Texture2D.h"

Texture2D::Texture2D(int width, int height, Texture2D::Format internalFormat, Texture2D::Format dataFormat,
                     const char *pixels)
        : m_internalFormat(internalFormat),
          m_width(width),
          m_height(height),
          m_id(0) {
    glGenTextures(1, &m_id);
    bind();

    glTexImage2D(GL_TEXTURE_2D,
                 0, // LoD level. Not very usefully because we can generate mipmaps.
                 m_internalFormat, // the internal Format of the texture
                 width, // Size of the input char*.
                 height,
                 0,  // := border. Must be zero.
                 dataFormat, // Format of the input char*.
                 GL_UNSIGNED_BYTE, // the pixels type of input char*.
                 pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    Texture2D::unbind();
}

Texture2D::Texture2D(ImageAsset &image, Texture2D::Format internalFormat) :
        Texture2D(image.width(), image.height(), internalFormat, [&]() {
            switch (image.channels()) {
                case 1:
                    return RED;
                case 2:
                    return RG;
                case 3:
                    return RGB;
                case 4:
                    return RGBA;
                default :
                    std::ostringstream ss;
                    ss << image.channels();
                    throw std::runtime_error("[Fatal Error] No Format defined for "
                                             + ss.str() + " channels");
            }
        }(), reinterpret_cast<const char *>(image.pixels())) { }
