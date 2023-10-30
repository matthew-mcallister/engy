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

class Image {
    std::vector<char> m_data;
    PixelFormat m_format;
    int m_width, m_height;

    Image() = default;

public:
    std::span<const char> data() const {
        return {m_data.begin(), m_data.end()};
    }
    PixelFormat format() const { return m_format; }
    int width() const { return m_width; }
    int height() const { return m_width; }

    static Image load(std::span<const char> data);
};

#endif