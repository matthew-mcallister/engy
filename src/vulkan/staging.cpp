#include "vulkan/staging.h"
#include "vulkan/memory.h"

StagingBuffer StagingBuffer::create(std::shared_ptr<VulkanAllocator> allocator,
                                    size_t size) {
    auto &device = allocator->device();

    vk::BufferCreateInfo buffer_info;
    buffer_info.size = size;
    buffer_info.usage = vk::BufferUsageFlagBits::eTransferSrc;

    VmaAllocationCreateInfo alloc_info;
    memset(&alloc_info, 0, sizeof(VmaAllocationCreateInfo));
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto buffer = VulkanAllocator::create_buffer(std::move(allocator),
                                                 buffer_info, alloc_info);

    auto semaphore = device.create_semaphore(vk::SemaphoreType::eTimeline);

    vk::CommandPoolCreateInfo pool_info;
    pool_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
    pool_info.queueFamilyIndex = 0;
    auto command_pool = device->createCommandPool(pool_info, nullptr);

    vk::CommandBufferAllocateInfo cmdbuf_info;
    cmdbuf_info.commandPool = *command_pool;
    cmdbuf_info.level = vk::CommandBufferLevel::ePrimary;
    cmdbuf_info.commandBufferCount = 1;
    auto cmd_buffers = device->allocateCommandBuffers(cmdbuf_info);

    return StagingBuffer(device, size, std::move(semaphore), std::move(buffer),
                         std::move(command_pool), std::move(cmd_buffers[0]));
}