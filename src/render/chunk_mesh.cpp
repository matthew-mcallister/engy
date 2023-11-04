#include <cassert>

#include <GL/gl.h>

#include "mesh_builder.h"
#include "render/chunk_mesh.h"

ChunkMesh::ChunkMesh() {
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    const size_t stride = sizeof(BlockVertex);
    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (GLvoid *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          (GLvoid *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, stride,
                          (GLvoid *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ChunkMesh::~ChunkMesh() {
    if (m_vao) {
        GLuint buffers[] = {
            m_vertex_buffer,
            m_index_buffer,
        };
        glDeleteBuffers(2, &buffers[0]);
        glDeleteVertexArrays(1, &m_vao);
    }
}

void ChunkMesh::update(const MeshData &data) {
    auto vertex_data = std::span(data.vertices);
    auto index_data = std::span(data.indices);

    if (vertex_data.size() == 0 || index_data.size() == 0) {
        return;
    }

    assert(vertex_data.size() > 0);
    assert(index_data.size() >= 6);
    assert(index_data.size() % 6 == 0);

    m_size = index_data.size() / 3;

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(BlockVertex),
                 &vertex_data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(uint32_t),
                 &index_data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ChunkMesh &ChunkMesh::operator=(ChunkMesh &&other) {
    m_vao = other.m_vao;
    m_vertex_buffer = other.m_vertex_buffer;
    m_index_buffer = other.m_index_buffer;
    other.m_vao = 0;
    other.m_vertex_buffer = 0;
    other.m_index_buffer = 0;
    return *this;
}
