#ifndef VULKAN_STAGING_H_INCLUDED
#define VULKAN_STAGING_H_INCLUDED

#include <vk_mem_alloc.h>

#include "vulkan/device.h"
#include "vulkan/memory.h"

class StagingBuffer {
    const VulkanDevice &m_device;
    const size_t m_size;
    const vk::raii::Semaphore m_semaphore;
    VulkanBuffer m_buffer;
    const vk::raii::CommandPool m_command_pool;
    const vk::raii::CommandBuffer m_command_buffer;

    StagingBuffer(const VulkanDevice &device, size_t size,
                  vk::raii::Semaphore semaphore, VulkanBuffer buffer,
                  vk::raii::CommandPool command_pool,
                  vk::raii::CommandBuffer command_buffer)
        : m_device{device}, m_size{size}, m_semaphore{std::move(semaphore)},
          m_buffer{std::move(buffer)}, m_command_pool{std::move(command_pool)},
          m_command_buffer{std::move(command_buffer)} {}

public:
    static StagingBuffer create(std::shared_ptr<VulkanAllocator> allocator,
                                size_t size);

    VulkanBuffer &buffer() { return m_buffer; }
    const VulkanBuffer &buffer() const { return m_buffer; }
};

#endif
