#include "block.h"

BlockRegistry BlockRegistry::create() {
    BlockRegistry registry;
    BlockInfo dirt = {
        BlockType::Dirt,
        {{"blocks/dirt.png"}, {"blocks/dirt.png"}, {"blocks/dirt.png"}},
        {true, true, true},
    };
    registry.add(BlockType::Dirt, std::move(dirt));
    return registry;
}

void BlockRegistry::add(BlockType type, BlockInfo &&info) {
    m_block_info.insert({type, std::move(info)});
}

const BlockInfo &BlockRegistry::get(BlockType type) const {
    return m_block_info.at(type);
}
