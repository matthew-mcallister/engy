#ifdef __INTELLISENSE__
#pragma diag_suppress 2361
#endif

#include <cassert>
#include <vector>

#include "chunk.h"

bool Block::is_solid() const {
    return type == block_type::SOLID;
}

ChunkMesh::ChunkMesh() {
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (GLvoid *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ChunkMesh::~ChunkMesh() {
    GLuint buffers[] = {
        m_vertex_buffer,
        m_index_buffer,
    };
    glDeleteBuffers(2, &buffers[0]);
    glDeleteVertexArrays(1, &m_vao);
}

void ChunkMesh::update(const MeshData &data) {
    auto vertex_data = std::span(data.vertices);
    auto index_data = std::span(data.indices);

    if (vertex_data.size() == 0 || index_data.size() == 0) {
        return;
    }

    assert(vertex_data.size() >= 4 * ChunkMesh::VERTEX_SIZE);
    assert(vertex_data.size() % ChunkMesh::VERTEX_SIZE == 0);
    assert(index_data.size() >= 6);
    assert(index_data.size() % 3 == 0);

    m_size = index_data.size() / 3;

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float),
                 &vertex_data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(int),
                 &index_data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ChunkMesh::draw() const {
    if (m_size == 0) {
        return;
    }
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 3 * m_size, GL_UNSIGNED_INT, nullptr);
}

auto Chunk::generate_mesh() const -> MeshData {
    std::vector<float> vertices;
    std::vector<int> indices;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            for (int k = 0; k < 7; k++) {
                const auto &block = m_data.blocks[i][j][k];

                const auto &x_neighbor = m_data.blocks[i + 1][j][k];
                if (block.is_solid() != x_neighbor.is_solid()) {
                    int i0 = vertices.size() / 6;
                    if (block.is_solid()) {
                        // clang-format off
                        vertices.insert(vertices.end(), {
                            i + 1, j,     k,     1, 0, 0,
                            i + 1, j + 1, k,     1, 0, 0,
                            i + 1, j,     k + 1, 1, 0, 0,
                            i + 1, j + 1, k + 1, 1, 0, 0,
                        });
                        indices.insert(indices.end(), {
                            i0, i0 + 1, i0 + 3,
                            i0, i0 + 3, i0 + 2,
                        });
                        // clang-format on
                    } else {
                        // clang-format off
                        vertices.insert(vertices.end(), {
                            i + 1, j,     k,     -1, 0, 0,
                            i + 1, j + 1, k,     -1, 0, 0,
                            i + 1, j,     k + 1, -1, 0, 0,
                            i + 1, j + 1, k + 1, -1, 0, 0,
                        });
                        indices.insert(indices.end(), {
                            i0, i0 + 3, i0 + 1,
                            i0, i0 + 2, i0 + 3,
                        });
                        // clang-format on
                    }
                }

                const auto &y_neighbor = m_data.blocks[i][j + 1][k];
                if (block.is_solid() != y_neighbor.is_solid()) {
                    int i0 = vertices.size() / 6;
                    if (block.is_solid()) {
                        // clang-format off
                        vertices.insert(vertices.end(), {
                            i,     j + 1, k,     0, 1, 0,
                            i + 1, j + 1, k,     0, 1, 0,
                            i,     j + 1, k + 1, 0, 1, 0,
                            i + 1, j + 1, k + 1, 0, 1, 0,
                        });
                        indices.insert(indices.end(), {
                            i0, i0 + 1, i0 + 3,
                            i0, i0 + 3, i0 + 2,
                        });
                        // clang-format on
                    } else {
                        // clang-format off
                        vertices.insert(vertices.end(), {
                            i,     j + 1, k,     0, -1, 0,
                            i + 1, j + 1, k,     0, -1, 0,
                            i,     j + 1, k + 1, 0, -1, 0,
                            i + 1, j + 1, k + 1, 0, -1, 0,
                        });
                        indices.insert(indices.end(), {
                            i0, i0 + 3, i0 + 1,
                            i0, i0 + 2, i0 + 3,
                        });
                        // clang-format on
                    }
                }

                const auto &z_neighbor = m_data.blocks[i][j][k + 1];
                if (block.is_solid() != z_neighbor.is_solid()) {
                    int i0 = vertices.size() / 6;
                    if (block.is_solid()) {
                        // clang-format off
                        vertices.insert(vertices.end(), {
                            i,     j,     k + 1, 0, 0, 1,
                            i + 1, j,     k + 1, 0, 0, 1,
                            i,     j + 1, k + 1, 0, 0, 1,
                            i + 1, j + 1, k + 1, 0, 0, 1,
                        });
                        indices.insert(indices.end(), {
                            i0, i0 + 1, i0 + 3,
                            i0, i0 + 3, i0 + 2,
                        });
                        // clang-format on
                    } else {
                        // clang-format off
                        vertices.insert(vertices.end(), {
                            i,     j,     k + 1, 0, 0, -1,
                            i + 1, j,     k + 1, 0, 0, -1,
                            i,     j + 1, k + 1, 0, 0, -1,
                            i + 1, j + 1, k + 1, 0, 0, -1,
                        });
                        indices.insert(indices.end(), {
                            i0, i0 + 3, i0 + 1,
                            i0, i0 + 2, i0 + 3,
                        });
                        // clang-format on
                    }
                }
            }
        }
    }
    return {
        vertices,
        indices,
    };
}

void Chunk::update_mesh() {
    const auto data = generate_mesh();
    m_mesh.update(data);
}

void Chunk::draw() const {
    m_mesh.draw();
}
