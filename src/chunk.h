#ifndef CHUNK_H_INCLUDED
#define CHUNK_H_INCLUDED

#include <optional>
#include <span>
#include <vector>

#include <GL/gl.h>

typedef int BlockType;

namespace block_type {

const BlockType EMPTY = 0;
const BlockType SOLID = 1;

} // namespace block_type

struct MeshData {
    std::vector<float> vertices;
    std::vector<int> indices;
};

struct Block {
    BlockType type;

    bool is_solid() const;
};

class ChunkMesh {
    GLuint m_vao;
    GLuint m_vertex_buffer;
    GLuint m_index_buffer;

    int m_size = 0;

public:
    ChunkMesh();
    ~ChunkMesh();

    // 3 position + 3 normal
    static const int VERTEX_SIZE = 6;

    void update(const MeshData &data);
    // Issues draw calls. Does not bind shaders or anything else.
    void draw() const;
};

struct ChunkData {
    Block blocks[8][8][8];
};

class Chunk {
    ChunkData m_data;
    ChunkMesh m_mesh;

    auto generate_mesh() const -> MeshData;

public:
    Chunk() = default;

    ChunkData &data() { return m_data; }
    const ChunkData &data() const { return m_data; }

    void update_mesh();
    void draw() const;
};

#endif