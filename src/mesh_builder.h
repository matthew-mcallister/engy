#ifndef MESH_BUILDER_H_INCLUDED
#define MESH_BUILDER_H_INCLUDED

#include <array>
#include <vector>

#include "block.h"
#include "chunk.h"
#include "render/texture_map.h"

enum class Axis {
    X,
    Y,
    Z,
};

enum class Direction {
    XPos,
    XNeg,
    YPos,
    YNeg,
    ZPos,
    ZNeg,
};

struct BlockFace {
    int i = 0;
    int j = 0;
    int k = 0;
    Direction dir = Direction::XPos;
    int rotation = 0;
    uint16_t texture = 0xffff;
};

struct BlockVertex {
    std::array<float, 3> pos;
    std::array<float, 3> normal;
    std::array<float, 2> texcoord;
    uint32_t texture;
};

static_assert(sizeof(BlockVertex) == 9 * sizeof(float));

struct MeshData {
    std::vector<BlockVertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t base_index() const;
    void add_face(const BlockFace &face);
};

class ChunkMeshBuilder {
    const BlockRegistry &m_block_registry;
    TextureMap &m_texture_map;
    std::vector<BlockFace> m_faces;

public:
    ChunkMeshBuilder(const BlockRegistry &registry, TextureMap &texture_map)
        : m_block_registry{registry}, m_texture_map{texture_map} {}

    void add_face(const BlockInfo &info, int i, int j, int k, Direction dir);
    // Neighbor is the adjacent block in the negative x, y, or z direction.
    void add_interface(const Block &block, const Block &neighbor, int i, int j,
                       int k, Axis axis);
    MeshData build();
};

auto generate_mesh(const BlockRegistry &block_registry, TextureMap &texture_map,
                   const ChunkMap &map, ChunkPos pos) -> MeshData;

#endif
