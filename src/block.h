#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <cstdint>
#include <string>
#include <unordered_map>

enum class BlockType {
    Empty = 0,
    Dirt = 1,
    Grass = 2,
};

struct BlockInfo {
    BlockType type;
    // top, middle, bottom
    std::string textures[3];
    bool rotate[3];
};

class BlockRegistry {
    std::unordered_map<BlockType, BlockInfo> m_block_info;

public:
    BlockRegistry() = default;

    static BlockRegistry create();

    void add(BlockInfo &&info);
    const BlockInfo &get(BlockType type) const;
};

struct Block {
    BlockType type;

    Block() = default;

    bool is_solid() const;
    void initialize();
};

#endif