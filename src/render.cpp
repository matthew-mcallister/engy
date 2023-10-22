#include <cmath>
#include <iostream>
#include <string>

#include <GL/gl.h>
#include <GL/glext.h>

#include "asset.h"
#include "config.h"
#include "math/matrix.h"
#include "math/scene.h"
#include "math/vector.h"
#include "render.h"

using namespace std::literals;
using std::numbers::pi;

// clang-format off
float VERTICES[] = {
    0, 0, 0, 1, 0, 0,
    1, 0, 0, 0, 1, 0,
    0.5, 0.867, 0, 0, 0, 1,
    0.5, 0.289, 0.816, 1, 1, 0,
};
int INDICES[] = {
    0, 1, 2,
    0, 3, 1,
    0, 3, 2,
    1, 3, 2
};
// clang-format on

struct ViewUniforms {
    Matrix4 projection;
    Matrix4 view;
};

Renderer::Renderer(AssetApi &assets) : m_assets(assets) {
    glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(0);
    glDepthFunc(GL_GEQUAL);
    // glClearDepth(1);
    // glDepthFunc(GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLuint depthRenderbuffer;
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, WINDOW_WIDTH,
                          WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthRenderbuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw SystemException("Failed to create framebuffer");
    }

    GLuint vertex_shader =
        compileShader("/shaders/vertex.glsl", GL_VERTEX_SHADER);
    GLuint fragment_shader =
        compileShader("/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    std::array<GLuint, 2> shaders{vertex_shader, fragment_shader};
    m_program = linkProgram(shaders);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (GLvoid *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES,
                 GL_STATIC_DRAW);

    glBindVertexArray(0);

    glGenBuffers(1, &m_uniform_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewUniforms), nullptr,
                 GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniform_buffer);
    glUniformBlockBinding(m_program, 0, 0);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Renderer::~Renderer() {
    glDeleteProgram(m_program);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vertex_buffer);
    glDeleteBuffers(1, &m_index_buffer);
    glDeleteBuffers(1, &m_uniform_buffer);
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
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Error in " << source_path << ": " << info
                  << std::endl; // TODO: Logging
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
        glGetProgramInfoLog(program, 512, nullptr, info);
        std::cerr << "Error linking shader program: " << info
                  << std::endl; // TODO: Logging
        throw DataException{"Failed to link shader program"};
    }

    return program;
}

Matrix4 get_view(float t) {
    auto center = vec3(0.5, 0.289, 0.204);
    auto theta = pi * t / 2;
    auto offset = vec3(cos(theta), sin(theta), 0.6);
    return scene::targeting_camera_xform(center + offset, center);
}

Matrix4 get_projection() {
    float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    return scene::projection(FOVY, aspect, Z_NEAR, Z_FAR);
}

void Renderer::render(float t) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view = get_view(t);
    auto proj = get_projection();
    ViewUniforms view_uniforms{proj, view};
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(view_uniforms),
                    &view_uniforms);

    glUseProgram(m_program);
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
}
