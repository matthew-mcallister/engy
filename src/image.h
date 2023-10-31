#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <iostream>
#include <span>
#include <vector>

enum class PixelFormat {
    R8,
    Rg8,
    Rgb8,
    Rgba8,
};

inline int format_size(PixelFormat format) {
    switch (format) {
    case PixelFormat::R8:
        return 1;
    case PixelFormat::Rg8:
        return 2;
    case PixelFormat::Rgb8:
        return 3;
    case PixelFormat::Rgba8:
        return 4;
    default:
        abort();
    }
}

inline GLuint format_to_gl(PixelFormat format) {
    switch (format) {
    case PixelFormat::R8:
        return GL_R;
    case PixelFormat::Rg8:
        return GL_RG;
    case PixelFormat::Rgb8:
        return GL_RGB;
    case PixelFormat::Rgba8:
        return GL_RGBA;
    default:
        abort();
    }
}

class Image {
    std::vector<char> m_data;
    PixelFormat m_format;
    int m_width, m_height;

    Image() = default;

    static Image _load(std::span<const char> data, bool as_rgba8);

public:
    std::span<const char> data() const {
        return {m_data.begin(), m_data.end()};
    }
    PixelFormat format() const { return m_format; }
    int width() const { return m_width; }
    int height() const { return m_width; }

    static Image load(std::span<const char> data);
    static Image load_rgba8(std::span<const char> data);
};

#endif