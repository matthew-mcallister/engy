#ifndef STAGING_H_INCLUDED
#define STAGING_H_INCLUDED

#include <vk_mem_alloc.h>

#include "vulkan/device.h"

class StagingBuffer {
    const VulkanDevice &m_device;

    const vk::raii::CommandPool m_command_pool;
    const vk::raii::CommandBuffer m_command_buffer;

public:
    StagingBuffer(VulkanDevice &device, VmaAllocator &allocator);
};

#endif
