#ifndef VULKAN_MESH_H_INCLUDED
#define VULKAN_MESH_H_INCLUDED

#include <optional>

#include "vulkan/memory.h"
#include "vulkan/staging.h"

class Mesh {
    VulkanBuffer m_vertex_buffer;
    VulkanBuffer m_index_buffer;

    // Size is the size of the index buffer
    uint32_t m_size = 0;
    uint64_t m_upload_batch = ~0;

public:
    Mesh(std::shared_ptr<VulkanAllocator> allocator, StagingBuffer &staging,
         std::span<const char> vertex_data,
         std::span<const uint32_t> index_data);

    uint32_t size() const { return m_size; }
    void bind(vk::raii::CommandBuffer &cmds) const;
};

#endif