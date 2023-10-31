#include <cassert>
#include <optional>

#include <GL/gl.h>
#include <GL/glext.h>

#include "render/texture_map.h"

const GLsizei SAMPLER_SIZE = 8;

TextureMapEntry::~TextureMapEntry() {
    if (gl_texture) {
        glDeleteTextures(1, &gl_texture);
    }
}

TextureMap::TextureMap() {
    glGenBuffers(1, &m_uniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniforms);
    glBufferData(GL_UNIFORM_BUFFER, SAMPLER_SIZE * TEXTURE_MAP_MAX_ENTRIES,
                 nullptr, GL_STATIC_DRAW);
}

TextureMap::~TextureMap() {
    if (m_uniforms) {
        glDeleteBuffers(1, &m_uniforms);
    }
}

TextureMapEntry &TextureMap::initialize(const std::string &path) {
    TextureMapEntry entry;
    auto id = m_entries.size();
    assert(id < TEXTURE_MAP_MAX_ENTRIES);
    entry.id = id;
    entry.path = std::move(path);
    m_entries.push_back(std::move(entry));
    m_entry_map.insert({path, entry.id});
    return m_entries[id];
}

std::optional<uint16_t> TextureMap::get(const std::string &path) {
    auto result = m_entry_map.find(path);
    if (result != m_entry_map.end()) {
        return m_entries[result->second].id;
    } else {
        return std::nullopt;
    }
}

uint16_t TextureMap::upload(const std::string &path, const Image &image) {
    TextureMapEntry &entry = initialize(path);
    assert(!entry.gl_texture);

    GLuint texture;
    glGenTextures(1, &texture);
    entry.gl_texture = texture;

    glBindTexture(GL_TEXTURE_2D, texture);
    auto gl_format = format_to_gl(image.format());
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, image.width(), image.height(), 0,
                 gl_format, GL_UNSIGNED_BYTE, &image.data()[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    entry.gl_bindless_handle = glGetTextureHandleARB(texture);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, m_uniforms);
    glBufferSubData(GL_UNIFORM_BUFFER, SAMPLER_SIZE * entry.id, SAMPLER_SIZE,
                    &entry.gl_bindless_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return entry.id;
}
