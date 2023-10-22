#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <span>

#include <GL/gl.h>

#include "asset.h"
#include "main.h"

class Renderer {
    AssetApi &m_assets;

    GLuint m_vertex_buffer;
    GLuint m_index_buffer;
    GLuint m_uniform_buffer;
    GLuint m_vao;
    GLuint m_program;

public:
    Renderer(AssetApi &assets);
    ~Renderer();

    void render(State &state);

private:
    auto compileShader(const char *source_path, GLuint type) -> GLuint;
    auto linkProgram(std::span<GLuint> shaders) -> GLuint;
};

#endif