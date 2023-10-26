#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <span>

#include <GL/gl.h>

#include "asset.h"
#include "chunk.h"
#include "main.h"

class Renderer {
    AssetApi &m_assets;

    GLuint m_uniform_buffer;
    GLuint m_program;

    auto compileShader(const char *source_path, GLuint type) -> GLuint;
    auto linkProgram(std::span<GLuint> shaders) -> GLuint;

public:
    Renderer(AssetApi &assets);
    ~Renderer();

    void prepare_frame(State &state);
    void render_chunk(const Chunk &chunk);
};

#endif