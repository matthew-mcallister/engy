#ifndef CHUNK_MESH_H_INCLUDED
#define CHUNK_MESH_H_INCLUDED

#include <utility>

#include <GL/gl.h>

class Renderer;
struct MeshData;
class TextureMap;

class ChunkMesh {
    GLuint m_vao;
    GLuint m_vertex_buffer;
    GLuint m_index_buffer;

    int m_size = 0;

    friend class Renderer;

public:
    ChunkMesh();
    ChunkMesh(const ChunkMesh &) = delete;
    ChunkMesh(ChunkMesh &&other) { *this = std::move(other); }
    ~ChunkMesh();

    ChunkMesh &operator=(ChunkMesh &&other);

    void update(const MeshData &data);
};

#endif
