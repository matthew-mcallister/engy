#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <cstdint>

enum class BlockType {
    Empty = 0,
    Solid = 1,
};

struct Block {
    BlockType type;
    // uint16_t textures[6];

    Block() = default;

    bool is_solid() const;
    void initialize();
};

#endif