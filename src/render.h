#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <cstdint>
#include <span>
#include <unordered_map>

#include <GL/gl.h>

#include "asset.h"
#include "chunk.h"
#include "image.h"
#include "main.h"
#include "render/texture_map.h"

class Renderer {
    AssetApi &m_assets;
    TextureMap m_texture_map;

    GLuint m_uniform_buffer;
    GLuint m_program;

    int m_instance_count = 0;

    auto compileShader(const char *source_path, GLuint type) -> GLuint;
    auto linkProgram(std::span<GLuint> shaders) -> GLuint;

public:
    Renderer(AssetApi &assets);
    ~Renderer();

    TextureMap &texture_map() { return m_texture_map; }

    uint16_t make_image_resident(const std::string &path);

    void prepare_frame(State &state);
    void render_chunk(const Chunk &chunk);
};

#endif