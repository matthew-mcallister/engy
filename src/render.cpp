#include <cmath>
#include <iomanip>
#include <iostream>
#include <span>
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
    // -z
    0, 0, 0, 0, 0, -1,
    1, 0, 0, 0, 0, -1,
    0, 1, 0, 0, 0, -1,
    1, 1, 0, 0, 0, -1,
    // +z
    0, 0, 1, 0, 0, 1,
    1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 0, 1,
    1, 1, 1, 0, 0, 1,
    // -y
    0, 0, 0, 0, -1, 0,
    1, 0, 0, 0, -1, 0,
    0, 0, 1, 0, -1, 0,
    1, 0, 1, 0, -1, 0,
    // +y
    0, 1, 0, 0, 1, 0,
    1, 1, 0, 0, 1, 0,
    0, 1, 1, 0, 1, 0,
    1, 1, 1, 0, 1, 0,
    // -x
    0, 0, 0, -1, 0, 0,
    0, 1, 0, -1, 0, 0,
    0, 0, 1, -1, 0, 0,
    0, 1, 1, -1, 0, 0,
    // +x
    1, 0, 0, 1, 0, 0,
    1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 0,
    1, 1, 1, 1, 0, 0,
};
int INDICES[] = {
    // -z
    0, 1, 2,
    0, 3, 1,
    0, 3, 2,
    1, 3, 2,
    // +z
    4, 5, 6,
    4, 7, 5,
    4, 7, 6,
    5, 7, 6,
    // -y
    8, 9,  10,
    8, 11, 9,
    8, 11, 10,
    9, 11, 10,
    // +y
    12, 13, 14,
    12, 15, 13,
    12, 15, 14,
    13, 15, 14,
    // -x
    16, 17, 18,
    16, 19, 17,
    16, 19, 18,
    17, 19, 18,
    // +x
    20, 21, 23,
    20, 22, 21,
    20, 22, 23,
    21, 22, 23,
};
// clang-format on

struct ViewUniforms {
    Matrix4 projection;
    Matrix4 view;
    Vector4 highlight;
};

struct Uniforms {
    ViewUniforms view_uniforms;
    Matrix4 instance[512];
};

Renderer::Renderer(AssetApi &assets) : m_assets(assets) {
    glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(0);
    glClearColor(0.3, 0.3, 0.3, 0.0);
    glDepthFunc(GL_GEQUAL);

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

    glGenBuffers(1, &m_uniform_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Uniforms), nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniform_buffer);
    glUniformBlockBinding(m_program, 0, 0);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Renderer::~Renderer() {
    glDeleteProgram(m_program);
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

Matrix4 get_projection() {
    float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    return scene::projection(FOVY, aspect, Z_NEAR, Z_FAR);
}

void Renderer::prepare_frame(State &state) {
    m_instance_count = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto proj = get_projection();
    auto view = state.rig().reverse_transform();
    Vector4 highlight;
    if (state.highlight()) {
        highlight = vec4(0.3, 0.15, 0, 0);
    }
    ViewUniforms view_uniforms{proj, view, highlight};
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(view_uniforms),
                    &view_uniforms);
    glUseProgram(m_program);
}

void Renderer::render_chunk(const Chunk &chunk) {
    auto instance = Matrix4::identity();
    instance[3] = chunk.pos().offset();
    glBufferSubData(GL_UNIFORM_BUFFER,
                    sizeof(ViewUniforms) + sizeof(Matrix4) * m_instance_count,
                    sizeof(Matrix4), &instance);
    chunk.draw(m_instance_count);
    m_instance_count++;
}
