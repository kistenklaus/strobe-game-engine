#include "ImageAsset.h"

#include "stb_image.h"
#include <cassert>
#include <stdexcept>

ImageAsset::ImageAsset(const std::string &path, int desiredChannels) :
        m_width(0), m_height(0), m_channels(0), m_pixels(nullptr) {
    m_pixels = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, desiredChannels);
    if (m_pixels == nullptr) {
        throw std::runtime_error("[Fatal Error] Failed to load Texture [" + path + "]");
    }
    assert(m_pixels != nullptr);
}

ImageAsset::~ImageAsset() {
    m_pixels = nullptr;
    m_width = 0;
    m_height = 0;
    m_channels = 0;
    stbi_image_free(m_pixels);
}
