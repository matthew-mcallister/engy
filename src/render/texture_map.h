#ifndef TEXTURE_MAP_H_INCLUDED
#define TEXTURE_MAP_H_INCLUDED

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/gl.h>

#include "image.h"

struct TextureMapEntry {
    uint16_t id = 0xffff;
    std::string path;
    GLuint gl_texture = 0;
    uint64_t gl_bindless_handle = 0;

    ~TextureMapEntry();
};

const size_t TEXTURE_MAP_MAX_ENTRIES = 8192;

class TextureMap {
    std::vector<TextureMapEntry> m_entries;
    std::unordered_map<std::string, uint16_t> m_entry_map;
    GLuint m_uniforms;

    TextureMapEntry &initialize(const std::string &path);

public:
    TextureMap();
    ~TextureMap();

    GLuint uniforms() const { return m_uniforms; }

    std::optional<uint16_t> get(const std::string &path);
    uint16_t upload(const std::string &path, const Image &image);
};

#endif
