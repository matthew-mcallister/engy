#include "block.h"

BlockRegistry BlockRegistry::create() {
    BlockRegistry registry;
    BlockInfo dirt = {
        BlockType::Dirt,
        {{"blocks/dirt.png"}, {"blocks/dirt.png"}, {"blocks/dirt.png"}},
        {true, true, true},
    };
    registry.add(std::move(dirt));
    BlockInfo grass = {
        BlockType::Grass,
        {{"blocks/grass_block_top.png"},
         {"blocks/grass_block_side.png"},
         {"blocks/dirt.png"}},
        {true, false, true},
    };
    registry.add(std::move(grass));
    return registry;
}

void BlockRegistry::add(BlockInfo &&info) {
    m_block_info.insert({info.type, std::move(info)});
}

const BlockInfo &BlockRegistry::get(BlockType type) const {
    return m_block_info.at(type);
}
