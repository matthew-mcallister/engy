#include <iostream>
#include <string>

#include <GL/gl.h>
#include <GL/glext.h>

#include "asset.h"
#include "render.h"

using namespace std::literals;

float VERTICES[] = {
    -0.5, 0.5, 0, 0.5, 0.5, 0, 0, -0.5, 0,
};

Renderer::Renderer(AssetApi &assets) : m_assets(assets) {
    glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    GLuint vertex_shader =
        compileShader("/shaders/vertex.glsl", GL_VERTEX_SHADER);
    GLuint fragment_shader =
        compileShader("/shaders/fragment.glsl", GL_FRAGMENT_SHADER);

    std::array<GLuint, 2> shaders{vertex_shader, fragment_shader};
    m_program = linkProgram(shaders);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Renderer::~Renderer() {
    glDeleteProgram(m_program);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vertex_buffer);
}

auto Renderer::compileShader(const char *source_path, GLuint type) -> GLuint {
    GLuint shader = glCreateShader(type);
    auto text = m_assets.load_text(source_path);
    auto cstr = text.c_str();
    int length = text.length();
    glShaderSource(shader, 1, &cstr, &length);
    glCompileShader(shader);

    int success;
    char info[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info);
        std::cerr << info << std::endl; // TODO: Logging
        throw DataException{"Failed to compile shader "s + source_path};
    }

    return shader;
}

auto Renderer::linkProgram(std::span<GLuint> shaders) -> GLuint {
    GLuint program = glCreateProgram();
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    int success;
    char info[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info);
        std::cerr << info << std::endl; // TODO: Logging
        throw DataException{"Failed to link shader program"};
    }

    return program;
}

void Renderer::render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_program);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
