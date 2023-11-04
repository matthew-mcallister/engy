#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

#include "block.h"
#include "chunk.h"
#include "mesh_builder.h"

using std::numbers::pi;

bool Block::is_solid() const {
    return type != BlockType::Empty;
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

void ChunkMap::update_mesh(const BlockRegistry &block_registry,
                           TextureMap &texture_map, ChunkPos pos) {
    const auto data = generate_mesh(block_registry, texture_map, *this, pos);
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
                    solid ? BlockType::Dirt : BlockType::Empty;
            }
        }
    }
}
