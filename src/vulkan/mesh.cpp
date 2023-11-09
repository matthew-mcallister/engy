#include "vulkan/mesh.h"

#include <util.h>

#include "exceptions.h"

VulkanBuffer create_buffer(std::shared_ptr<VulkanAllocator> allocator,
                           vk::DeviceSize size, vk::BufferUsageFlagBits usage) {
    const uint32_t index = 0;
    vk::BufferCreateInfo buf_info;
    buf_info.size = size;
    buf_info.usage = usage | vk::BufferUsageFlagBits::eTransferDst;
    buf_info.setQueueFamilyIndices(index);
    VmaAllocationCreateInfo alloc_info;
    memset(&alloc_info, 0, sizeof(VmaAllocationCreateInfo));
    alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
    return VulkanAllocator::create_buffer(allocator, buf_info, alloc_info);
}

Mesh::Mesh(std::shared_ptr<VulkanAllocator> allocator, StagingBuffer &staging,
           std::span<const char> vertex_data,
           std::span<const uint32_t> index_data)
    : m_vertex_buffer{create_buffer(allocator, vertex_data.size(),
                                    vk::BufferUsageFlagBits::eVertexBuffer)},
      m_index_buffer{create_buffer(allocator,
                                   sizeof(uint32_t) * index_data.size(),
                                   vk::BufferUsageFlagBits::eIndexBuffer)},
      m_size{index_data.size()} {
    const auto index_bytes = as_bytes(index_data);
    if (vertex_data.size() + index_bytes.size() > staging.remaining()) {
        throw OutOfMemoryException("Staging buffer full");
    }
    staging.stage_buffer(vertex_data, m_vertex_buffer, 0);
    m_upload_batch = staging.stage_buffer(index_bytes, m_index_buffer, 0);
}

void Mesh::bind(vk::raii::CommandBuffer &cmds) const {
    const vk::DeviceSize offset = 0;
    const vk::DeviceSize size = m_vertex_buffer.size();
    cmds.bindVertexBuffers2(0, *m_vertex_buffer, offset, size, nullptr);
    cmds.bindIndexBuffer(*m_index_buffer, 0, vk::IndexType::eUint32);
}
