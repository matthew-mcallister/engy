#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <iostream>
#include <span>
#include <vector>

#include <vulkan/vulkan.hpp>

enum class PixelFormat {
    R8,
    Rg8,
    Rgb8,
    Rgba8,
};

inline uint32_t format_size(PixelFormat format) {
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

inline vk::Format format_to_vk(PixelFormat format) {
    // Sigh have to detect image color encoding from file. Cannot use
    // sRGB for e.g. normal maps.
    switch (format) {
    case PixelFormat::R8:
        return vk::Format::eR8Srgb;
    case PixelFormat::Rg8:
        return vk::Format::eR8G8Srgb;
    case PixelFormat::Rgb8:
        return vk::Format::eR8G8B8Srgb;
    case PixelFormat::Rgba8:
        return vk::Format::eR8G8B8A8Srgb;
    default:
        abort();
    }
}

inline PixelFormat vk_to_format(vk::Format format) {
    switch (format) {
    case vk::Format::eR8Srgb:
        return PixelFormat::R8;
    case vk::Format::eR8G8Srgb:
        return PixelFormat::Rg8;
    case vk::Format::eR8G8B8Srgb:
        return PixelFormat::Rgb8;
    case vk::Format::eR8G8B8A8Srgb:
        return PixelFormat::Rgba8;
    default:
        abort();
    }
}

class Image {
    std::vector<char> m_data;
    PixelFormat m_format;
    uint32_t m_width, m_height;

    Image() = default;

    static Image _load(std::span<const char> data, bool as_rgba8);

public:
    Image(const Image &other) = delete;
    Image(Image &&other) = default;

    std::span<const char> data() const {
        return {m_data.begin(), m_data.end()};
    }
    PixelFormat format() const { return m_format; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_width; }

    static Image load(std::span<const char> data);
};

#endif