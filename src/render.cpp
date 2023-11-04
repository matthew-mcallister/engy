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

struct ViewUniforms {
    Matrix4 projection;
    Matrix4 view;
    Vector4 highlight;
};

struct Uniforms {
    ViewUniforms view_uniforms;
    Matrix4 instance[512];
};

auto gl_severity(GLenum severity) -> const char * {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        return "high";
    case GL_DEBUG_SEVERITY_MEDIUM:
        return "medium";
    case GL_DEBUG_SEVERITY_LOW:
        return "low";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "notification";
    default:
        return "unknown";
    }
}

auto gl_message_type(GLenum type) -> const char * {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        return "error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "deprecation";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "undefined_behavior";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "portability";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "performance";
    case GL_DEBUG_TYPE_MARKER:
        return "marker";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "push";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "pop";
    case GL_DEBUG_TYPE_OTHER:
        return "other";
    default:
        return "unknown";
    }
}

auto gl_message_source(GLenum source) -> const char * {
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        return "api";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "window_system";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "shader";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "3rd_party";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "application";
    case GL_DEBUG_SOURCE_OTHER:
        return "other";
    default:
        return "unknown";
    }
}

void GLAPIENTRY debug_handler(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
    std::cout << "[" << id << ":" << gl_message_source(source) << ":"
              << gl_message_type(type) << ":" << gl_severity(severity) << "] "
              << message << std::endl;
}

Renderer::Renderer(AssetApi &assets) : m_assets(assets) {
    if (m_debug) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debug_handler, nullptr);
    }

    glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(0);
    glClearColor(0.3, 0.3, 0.3, 0.0);
    glDepthFunc(GL_GEQUAL);

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

    auto texture_map = m_texture_map.uniforms();
    glBindBuffer(GL_UNIFORM_BUFFER, texture_map);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, texture_map);
    glUniformBlockBinding(m_program, 1, 1);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
    if (!chunk.mesh()) {
        return;
    }
    auto instance = Matrix4::identity();
    instance[3] = chunk.pos().offset();
    render_mesh(*chunk.mesh(), instance);
}

void Renderer::render_mesh(const ChunkMesh &mesh, Matrix4 instance) {
    if (mesh.m_size == 0) {
        return;
    }
    glBufferSubData(GL_UNIFORM_BUFFER,
                    sizeof(ViewUniforms) + sizeof(Matrix4) * m_instance_count,
                    sizeof(Matrix4), &instance);
    glBindVertexArray(mesh.m_vao);
    glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 3 * mesh.m_size,
                                        GL_UNSIGNED_INT, nullptr, 1,
                                        m_instance_count);
    m_instance_count++;
}

uint16_t Renderer::make_image_resident(const std::string &path) {
    const auto result = m_texture_map.get({path});
    if (result) {
        return *result;
    }

    auto image = Image::load(m_assets.load_blob(path));
    return m_texture_map.upload(path, image);
}
