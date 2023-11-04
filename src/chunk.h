#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED

#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "block.h"
#include "math/vector.h"
#include "render/chunk_mesh.h"

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
    std::optional<ChunkMesh> m_mesh;
    bool m_generated = false;

public:
    Chunk() {}

    ChunkPos &pos() { return m_pos; }
    const ChunkPos &pos() const { return m_pos; }

    ChunkData &data() { return m_data; }
    const ChunkData &data() const { return m_data; }

    const std::optional<ChunkMesh> &mesh() const { return m_mesh; }

    void update_mesh(const MeshData &data);
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
                     TextureMap &texture_map, ChunkPos pos);
};

#endif