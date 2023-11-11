#include "vulkan/staging.h"

#include <cstring>
#include <tuple>

#include "exceptions.h"
#include "image.h"
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

void StagingBuffer::begin_staging() {
    assert(!m_staging);
    m_staging = true;
    m_command_pool.reset();
    vk::CommandBufferBeginInfo info;
    info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    m_command_buffer.begin(info);
}

uint64_t StagingBuffer::stage_buffer(std::span<const char> data,
                                     VulkanBuffer &dest,
                                     vk::DeviceSize offset) {
    assert(m_staging);
    if (m_offset + data.size() > m_size) {
        throw OutOfMemoryException("Staging buffer out of memory");
    }

    // Copy to staging buffer
    std::memcpy((void *)((const char *)m_buffer.data() + m_offset), data.data(),
                data.size());

    // Copy from staging to destination buffer
    vk::BufferCopy2 copy;
    copy.srcOffset = m_offset;
    copy.dstOffset = offset;
    copy.size = data.size();
    vk::CopyBufferInfo2 info;
    info.srcBuffer = *m_buffer;
    info.dstBuffer = *dest;
    info.setRegions(copy);
    m_command_buffer.copyBuffer2(info);

    m_offset += data.size();
    return m_pending_batch + 1;
}

uint64_t StagingBuffer::stage_image(const Image &src, VulkanImage &dest,
                                    bool generate_mipmaps) {
    assert(m_staging);
    const auto data = src.data();
    assert(dest.width() == src.width() && dest.height() == src.height() &&
           dest.depth() == 1);
    assert(src.format() == vk_to_format(dest.format()));
    assert(dest.array_layers() == 1);
    if (m_offset + data.size() > m_size) {
        throw OutOfMemoryException("Staging buffer out of memory");
    }

    // Step 1: Write to staging buffer
    std::memcpy((void *)((const char *)m_buffer.data() + m_offset), data.data(),
                data.size());

    vk::ImageSubresourceRange range;
    range.aspectMask = vk::ImageAspectFlagBits::eColor;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    // Step 2: Transition to TransferDstOptimal
    {
        vk::ImageMemoryBarrier2 img_barrier;
        img_barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
        img_barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
        img_barrier.oldLayout = vk::ImageLayout::eUndefined;
        img_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        img_barrier.image = *dest;
        img_barrier.subresourceRange = range;
        vk::DependencyInfo dep;
        dep.setImageMemoryBarriers(img_barrier);
        m_command_buffer.pipelineBarrier2(dep);
    }

    // Step 3: Copy from staging to image
    vk::BufferImageCopy2 copy;
    copy.bufferOffset = m_offset;
    copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    copy.imageSubresource.mipLevel = 0;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent = dest.extent();
    vk::CopyBufferToImageInfo2 info;
    info.srcBuffer = *m_buffer;
    info.dstImage = *dest;
    info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
    info.setRegions(copy);
    m_command_buffer.copyBufferToImage2(info);

    // Step 4: Create mipmaps
    // Step 4.1: Transition to TransferSrcOptimal
    // Step 4.2: Copy to mip levels
    assert(!generate_mipmaps);

    // Step 6: Transition to ShaderReadOnlyOptimal
    {
        vk::ImageMemoryBarrier2 img_barrier;
        img_barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
        img_barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        img_barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
        img_barrier.dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead;
        img_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        img_barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        img_barrier.image = *dest;
        img_barrier.subresourceRange = range;
        vk::DependencyInfo dep;
        dep.setImageMemoryBarriers(img_barrier);
        m_command_buffer.pipelineBarrier2(dep);
    }

    m_offset += data.size();
    return m_pending_batch + 1;
}

void StagingBuffer::end_staging(vk::raii::Queue &queue) {
    assert(m_staging);
    m_staging = false;
    m_command_buffer.end();
    m_pending_batch++;

    vk::CommandBufferSubmitInfo cmd_info;
    cmd_info.commandBuffer = *m_command_buffer;
    vk::SemaphoreSubmitInfo sem_info;
    sem_info.semaphore = *m_semaphore;
    sem_info.value = m_pending_batch;
    sem_info.stageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    vk::SubmitInfo2 info;
    info.setCommandBufferInfos(cmd_info);
    info.setSignalSemaphoreInfos(sem_info);
    queue.submit2(info);
}

void StagingBuffer::wait() const {
    vk::SemaphoreWaitInfo info;
    info.setSemaphores(*m_semaphore);
    info.setValues(m_pending_batch);
    std::ignore = m_device->waitSemaphores(info, 1'000'000'000);
}
