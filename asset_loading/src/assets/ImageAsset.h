#pragma once

#include <string>

class ImageAsset {
public:
    explicit ImageAsset(const std::string &path, int desiredChannels = 0);

    ~ImageAsset();

    [[nodiscard]] inline int width() const { return m_width; }

    [[nodiscard]] inline int height() const { return m_height; }

    [[nodiscard]] inline int channels() const { return m_channels; }

    [[nodiscard]] inline const unsigned char *pixels() const { return m_pixels; }

private:
    int m_width;
    int m_height;
    int m_channels;
    unsigned char *m_pixels;
};
