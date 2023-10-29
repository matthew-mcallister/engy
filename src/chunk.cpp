#ifdef __INTELLISENSE__
#pragma diag_suppress 2361
#endif

#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

#include "block.h"
#include "chunk.h"

using std::numbers::pi;

bool Block::is_solid() const {
    return type != BlockType::Empty;
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

void ChunkMesh::draw(int instance_id) const {
    if (m_size == 0) {
        return;
    }
    glBindVertexArray(m_vao);
    glDrawElementsInstancedBaseInstance(
        GL_TRIANGLES, 3 * m_size, GL_UNSIGNED_INT, nullptr, 1, instance_id);
}

void Chunk::update_mesh(const MeshData &data) {
    if (data.vertices.empty()) {
        m_mesh.reset();
        return;
    }
    if (!m_mesh) {
        m_mesh = ChunkMesh();
    }
    m_mesh->update(data);
}

void Chunk::draw(int instance_id) const {
    if (m_mesh) {
        m_mesh->draw(instance_id);
    }
}

struct ChunkMeshBuilder {
    std::vector<float> vertices;
    std::vector<int> indices;

    ChunkMeshBuilder() = default;

    int base_index() const { return vertices.size() / 6; }

    void x_face_pos(int i, int j, int k) {
        int i0 = base_index();
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
    }

    void x_face_neg(int i, int j, int k) {
        int i0 = base_index();
        // clang-format off
        vertices.insert(vertices.end(), {
            i + 1, j,     k,     -1, 0, 0,
            i + 1, j + 1, k,     -1, 0, 0,
            i + 1, j,     k + 1, -1, 0, 0,
            i + 1, j + 1, k + 1, -1, 0, 0,
        });
        indices.insert(indices.end(), {
            i0, i0 + 1, i0 + 3,
            i0, i0 + 3, i0 + 2,
        });
        // clang-format on
    }

    void y_face_pos(int i, int j, int k) {
        int i0 = base_index();
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
    }

    void y_face_neg(int i, int j, int k) {
        int i0 = base_index();
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

    void z_face_pos(int i, int j, int k) {
        int i0 = base_index();
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
    }

    void z_face_neg(int i, int j, int k) {
        int i0 = base_index();
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
};

auto generate_mesh(ChunkMap &map, ChunkPos pos) -> MeshData {
    ChunkMeshBuilder builder;
    const Chunk &chunk = map[pos];
    const auto &blocks = chunk.data().blocks;

    // YZ
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                const auto &block = blocks[i][j][k];
                const auto &x_neighbor = blocks[i + 1][j][k];
                if (block.is_solid() != x_neighbor.is_solid()) {
                    if (block.is_solid()) {
                        builder.x_face_pos(i, j, k);
                    } else {
                        builder.x_face_neg(i, j, k);
                    }
                }
            }
        }
    }
    const auto &x_neighbor_blocks =
        map[{pos.i - 1, pos.j, pos.k}].data().blocks;
    for (int j = 0; j < 8; j++) {
        for (int k = 0; k < 8; k++) {
            const auto &block = x_neighbor_blocks[7][j][k];
            const auto &x_neighbor = blocks[0][j][k];
            if (block.is_solid() != x_neighbor.is_solid()) {
                if (block.is_solid()) {
                    builder.x_face_pos(-1, j, k);
                } else {
                    builder.x_face_neg(-1, j, k);
                }
            }
        }
    }

    // ZX
    for (int j = 0; j < 7; j++) {
        for (int i = 0; i < 8; i++) {
            for (int k = 0; k < 8; k++) {
                const auto &block = blocks[i][j][k];
                const auto &y_neighbor = blocks[i][j + 1][k];
                if (block.is_solid() != y_neighbor.is_solid()) {
                    if (block.is_solid()) {
                        builder.y_face_pos(i, j, k);
                    } else {
                        builder.y_face_neg(i, j, k);
                    }
                }
            }
        }
    }
    const auto &y_neighbor_blocks =
        map[{pos.i, pos.j - 1, pos.k}].data().blocks;
    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < 8; k++) {
            const auto &block = y_neighbor_blocks[i][7][k];
            const auto &y_neighbor = blocks[i][0][k];
            if (block.is_solid() != y_neighbor.is_solid()) {
                if (block.is_solid()) {
                    builder.y_face_pos(i, -1, k);
                } else {
                    builder.y_face_neg(i, -1, k);
                }
            }
        }
    }

    // XY
    for (int k = 0; k < 7; k++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                const auto &block = blocks[i][j][k];
                const auto &z_neighbor = blocks[i][j][k + 1];
                if (block.is_solid() != z_neighbor.is_solid()) {
                    if (block.is_solid()) {
                        builder.z_face_pos(i, j, k);
                    } else {
                        builder.z_face_neg(i, j, k);
                    }
                }
            }
        }
    }
    const auto &z_neighbor_blocks =
        map[{pos.i, pos.j, pos.k - 1}].data().blocks;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            const auto &block = z_neighbor_blocks[i][j][7];
            const auto &z_neighbor = blocks[i][j][0];
            if (block.is_solid() != z_neighbor.is_solid()) {
                if (block.is_solid()) {
                    builder.z_face_pos(i, j, -1);
                } else {
                    builder.z_face_neg(i, j, -1);
                }
            }
        }
    }

    return {
        std::move(builder.vertices),
        std::move(builder.indices),
    };
}

void ChunkMap::update_mesh(ChunkPos pos) {
    const auto data = generate_mesh(*this, pos);
    (*this)[pos].update_mesh(data);
}

float f(float x, float y) {
    return 4 + sinf(pi * x / 2) + sinf(pi * y / 2);
}

void ChunkMap::generate_chunk(ChunkPos pos) {
    auto &chunk = (*this)[pos];
    chunk.pos() = pos;
    auto &blocks = chunk.data().blocks;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                bool solid = 8 * pos.k + k <= f(i, j);
                blocks[i][j][k].type =
                    solid ? BlockType::Solid : BlockType::Empty;
            }
        }
    }
}
