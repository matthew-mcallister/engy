#ifdef __INTELLISENSE__
#pragma diag_suppress 2361
#endif

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "mesh_builder.h"

std::array<float, 3> direction_normal(Direction dir) {
    if (dir == Direction::XPos) {
        return {1, 0, 0};
    } else if (dir == Direction::XNeg) {
        return {-1, 0, 0};
    } else if (dir == Direction::YPos) {
        return {0, 1, 0};
    } else if (dir == Direction::YNeg) {
        return {0, -1, 0};
    } else if (dir == Direction::ZPos) {
        return {0, 0, 1};
    } else {
        return {0, 0, -1};
    }
}

Direction axis_to_dir_pos(Axis axis) {
    switch (axis) {
    case Axis::X:
        return Direction::XPos;
    case Axis::Y:
        return Direction::YPos;
    case Axis::Z:
        return Direction::ZPos;
    default:
        assert(false);
    }
}

Direction axis_to_dir_neg(Axis axis) {
    switch (axis) {
    case Axis::X:
        return Direction::XNeg;
    case Axis::Y:
        return Direction::YNeg;
    case Axis::Z:
        return Direction::ZNeg;
    default:
        assert(false);
    }
}

void MeshData::add_face(const BlockFace &face) {
    uint32_t i0 = vertices.size();
    int i = face.i, j = face.j, k = face.k;

    BlockVertex vs[4];
    switch (face.dir) {
    case Direction::XPos:
    case Direction::XNeg:
        // clang-format off
        vs[0].pos = {i, j,     k    };
        vs[1].pos = {i, j + 1, k    };
        vs[2].pos = {i, j + 1, k + 1};
        vs[3].pos = {i, j,     k + 1};
        // clang-format on
        break;
    case Direction::YPos:
    case Direction::YNeg:
        // clang-format off
        vs[0].pos = {i,     j, k    };
        vs[1].pos = {i + 1, j, k    };
        vs[2].pos = {i + 1, j, k + 1};
        vs[3].pos = {i,     j, k + 1};
        // clang-format on
        break;
    case Direction::ZPos:
    case Direction::ZNeg:
        // clang-format off
        vs[0].pos = {i,     j,     k};
        vs[1].pos = {i + 1, j,     k};
        vs[2].pos = {i + 1, j + 1, k};
        vs[3].pos = {i,     j + 1, k};
        // clang-format on
        break;
    }

    auto normal = direction_normal(face.dir);
    for (auto &v : vs) {
        v.normal = normal;
        v.texture = face.texture;
    }

    if (face.dir == Direction::XNeg || face.dir == Direction::YNeg ||
        face.dir == Direction::ZNeg) {
        // clang-format off
        indices.insert(indices.end(), {
            i0, i0 + 1, i0 + 2,
            i0, i0 + 2, i0 + 3,
        });
        // clang-format on
    } else {
        // clang-format off
        indices.insert(indices.end(), {
            i0, i0 + 2, i0 + 1,
            i0, i0 + 3, i0 + 2,
        });
        // clang-format on
    }

    std::array<float, 2> tex_coords[4] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1},
    };
    for (int i = 0; i < 4; i++) {
        vs[i].texcoord = tex_coords[(i + face.rotation) % 4];
    }

    vertices.push_back(vs[0]);
    vertices.push_back(vs[1]);
    vertices.push_back(vs[2]);
    vertices.push_back(vs[3]);
}

void ChunkMeshBuilder::add_face(const BlockInfo &info, int i, int j, int k,
                                Direction dir) {
    BlockFace face;
    face.i = i;
    face.j = j;
    face.k = k;
    face.dir = dir;

    int index = 0;
    if (dir == Direction::ZPos) {
        index = 0;
    } else if (dir == Direction::ZNeg) {
        index = 2;
    } else {
        index = 1;
    }

    face.texture = m_texture_map.get(info.textures[index]);

    if (info.rotate[index]) {
        face.rotation = rand() % 4;
    }

    m_faces.push_back(face);
}

void ChunkMeshBuilder::add_interface(const Block &block, const Block &neighbor,
                                     int i, int j, int k, Axis axis) {
    if (block.is_solid() == neighbor.is_solid()) {
        return;
    }

    Direction dir;
    const BlockInfo *info;
    if (block.is_solid()) {
        dir = axis_to_dir_neg(axis);
        info = &m_block_registry.get(block.type);
    } else {
        dir = axis_to_dir_pos(axis);
        info = &m_block_registry.get(neighbor.type);
    }

    add_face(*info, i, j, k, dir);
}

MeshData ChunkMeshBuilder::build() {
    // Sort by texture for cache optimization
    const auto cmp = [](const auto &f1, const auto &f2) {
        return f1.texture < f2.texture;
    };
    std::sort(m_faces.begin(), m_faces.end(), cmp);

    MeshData data;
    for (const auto &face : m_faces) {
        data.add_face(face);
    }

    return data;
};

auto generate_mesh(const BlockRegistry &block_registry, TextureMap &texture_map,
                   const ChunkMap &map, ChunkPos pos) -> MeshData {
    ChunkMeshBuilder builder{block_registry, texture_map};
    const Chunk *chunks[4] = {
        &map.at(pos),
        &map.at({pos.i - 1, pos.j, pos.k}),
        &map.at({pos.i, pos.j - 1, pos.k}),
        &map.at({pos.i, pos.j, pos.k - 1}),
    };
    const auto get_block = [chunks](int i, int j, int k) -> const Block & {
        // Some indexing trickery so we don't call map.at for every single block
        unsigned int chunk_index = -(i >> 3) - 2 * (j >> 3) - 3 * (k >> 3);
        assert(chunk_index < 4);
        const auto &blocks = chunks[chunk_index]->data().blocks;
        return blocks[(i + 8) % 8][(j + 8) % 8][(k + 8) % 8];
    };
    for (const Chunk *chunk : chunks) {
        assert(chunk->generated());
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                const auto &block = get_block(i, j, k);
                builder.add_interface(block, get_block(i - 1, j, k), i, j, k,
                                      Axis::X);
                builder.add_interface(block, get_block(i, j - 1, k), i, j, k,
                                      Axis::Y);
                builder.add_interface(block, get_block(i, j, k - 1), i, j, k,
                                      Axis::Z);
            }
        }
    }

    return builder.build();
}
