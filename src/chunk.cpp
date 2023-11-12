#include <cassert>
#include <cmath>
#include <numbers>
#include <span>
#include <vector>

#include "block.h"
#include "chunk.h"
#include "mesh_builder.h"
#include "util.h"
#include "vulkan/renderer.h"

using std::numbers::pi;

bool Block::is_solid() const {
    return type != BlockType::Empty;
}

void Chunk::update_mesh(VulkanRenderer &renderer, const MeshData &data) {
    m_mesh.reset();
    if (data.vertices.empty() || data.indices.empty()) {
        return;
    }
    m_mesh =
        renderer.create_mesh(as_bytes(std::span{data.vertices}), data.indices);
}

void ChunkMap::update_mesh(const BlockRegistry &block_registry,
                           VulkanRenderer &renderer, ChunkPos pos) {
    const auto data =
        generate_mesh(block_registry, renderer.textures(), *this, pos);
    (*this)[pos].update_mesh(renderer, data);
}

float f(float x, float y) {
    return 4 + sinf(pi * x / 2) + sinf(pi * y / 2);
}

void ChunkMap::generate_chunk(ChunkPos pos) {
    auto &chunk = (*this)[pos];
    if (chunk.m_generated) {
        return;
    }
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
    chunk.m_generated = true;
}
