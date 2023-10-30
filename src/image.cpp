#include <cstdint>

#include <spng.h>

#include "exceptions.h"
#include "image.h"

struct SpngCtx {
    spng_ctx *inner;

    SpngCtx() { inner = spng_ctx_new(0); }
    ~SpngCtx() { spng_ctx_free(inner); }
};

PixelFormat select_format(uint8_t png_type) {
    switch (png_type) {
    case 0:
        return PixelFormat::R8;
    case 2:
        return PixelFormat::Rgb8;
    case 4:
        // Converting greyscale-with-alpha to Rg8 would be too confusing
        return PixelFormat::Rgba8;
    case 3:
    case 6:
        return PixelFormat::Rgba8;
    default:
        abort();
    }
}

spng_format select_spng_format(uint8_t png_type) {
    switch (png_type) {
    case 0:
        return SPNG_FMT_G8;
    case 2:
        return SPNG_FMT_RGB8;
    case 4:
        return SPNG_FMT_RGBA8;
    case 3:
    case 6:
        return SPNG_FMT_RGBA8;
    default:
        abort();
    }
}

auto Image::load(std::span<const char> data) -> Image {
    SpngCtx ctx;
    spng_set_png_buffer(ctx.inner, &data[0], data.size());

    spng_ihdr ihdr;
    if (spng_get_ihdr(ctx.inner, &ihdr)) {
        throw DataException("Invalid PNG");
    }

    int width = ihdr.width, height = ihdr.height;
    auto pixel_format = select_format(ihdr.color_type);
    auto spng_format = select_spng_format(ihdr.color_type);

    size_t size;
    spng_decoded_image_size(ctx.inner, spng_format, &size);
    std::vector<char> bytes;
    bytes.resize(size);
    spng_decode_image(ctx.inner, &bytes[0], size, spng_format, 0);

    Image result;
    result.m_width = width;
    result.m_height = height;
    result.m_data = std::move(bytes);
    result.m_format = pixel_format;
    return result;
}
