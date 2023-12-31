#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED

#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "block.h"
#include "math/vector.h"
#include "vulkan/mesh.h"
#include "vulkan/renderer.h"
#include "vulkan/staging.h"
#include "vulkan/texture_map.h"

class MeshData;

struct ChunkPos {
    int i;
    int j;
    int k;

    bool operator==(const ChunkPos &other) const {
        return i == other.i && j == other.j && k == other.k;
    }

    Vector4 offset() const { return vec4(8 * i, 8 * j, 8 * k, 1); }
};

namespace std {

template<>
struct hash<ChunkPos> {
    inline size_t operator()(const ChunkPos &x) const {
        const auto view =
            std::string_view((const char *)(&x), sizeof(ChunkPos));
        return std::hash<std::string_view>()(view);
    }
};

} // namespace std

struct ChunkData {
    Block blocks[8][8][8];
};

class Chunk {
    ChunkPos m_pos;
    ChunkData m_data;
    std::optional<Mesh> m_mesh;
    bool m_generated = false;

    friend class ChunkMap;

public:
    Chunk() {}

    ChunkPos &pos() { return m_pos; }
    const ChunkPos &pos() const { return m_pos; }

    ChunkData &data() { return m_data; }
    const ChunkData &data() const { return m_data; }
    const std::optional<Mesh> &mesh() const { return m_mesh; }
    bool generated() const { return m_generated; }

    void update_mesh(VulkanRenderer &renderer, const MeshData &data);
};

class ChunkMap {
    std::unordered_map<ChunkPos, Chunk> m_chunks;

public:
    ChunkMap() {}

    Chunk &at(ChunkPos pos) { return m_chunks.at(pos); }
    const Chunk &at(ChunkPos pos) const { return m_chunks.at(pos); }
    Chunk &operator[](ChunkPos pos) { return m_chunks[pos]; }

    void generate_chunk(ChunkPos pos);
    void update_mesh(const BlockRegistry &block_registry,
                     VulkanRenderer &renderer, ChunkPos pos);
};

#endif