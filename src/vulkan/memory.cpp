#include "vulkan/memory.h"

#include "vulkan/device.h"

VulkanAllocator::VulkanAllocator(VulkanDevice &device,
                                 const VmaAllocatorCreateInfo &create_info)
    : m_device{device} {
    vmaCreateAllocator(&create_info, &m_allocator);
}

VulkanAllocator::~VulkanAllocator() {
    if (m_allocator) {
        vmaDestroyAllocator(m_allocator);
    }
}

VulkanBuffer VulkanAllocator::create_buffer(
    std::shared_ptr<VulkanAllocator> allocator,
    const vk::BufferCreateInfo &buffer_create_info,
    const VmaAllocationCreateInfo &allocation_create_info) {
    VkBuffer vk_buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    vmaCreateBuffer(allocator->m_allocator,
                    (VkBufferCreateInfo *)&buffer_create_info,
                    &allocation_create_info, &vk_buffer, &allocation, &info);
    vk::raii::Buffer buffer{*allocator->m_device, vk_buffer};
    return VulkanBuffer(std::move(allocator), allocation, info,
                        std::move(buffer));
}

VulkanImage VulkanAllocator::create_image(
    std::shared_ptr<VulkanAllocator> allocator,
    const vk::ImageCreateInfo &image_create_info,
    const VmaAllocationCreateInfo &allocation_create_info) {
    VkImage vk_image;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    auto result = vmaCreateImage(
        allocator->m_allocator, (VkImageCreateInfo *)&image_create_info,
        &allocation_create_info, &vk_image, &allocation, &info);
    assert(result == VK_SUCCESS);
    vk::raii::Image image{*allocator->m_device, vk_image};
    return VulkanImage(std::move(allocator), allocation, info, std::move(image),
                       image_create_info.extent, image_create_info.mipLevels,
                       image_create_info.arrayLayers, image_create_info.format);
}

vk::ImageAspectFlags all_aspects(vk::Format format) {
    switch (format) {
    case vk::Format::eD16Unorm:
    case vk::Format::eD32Sfloat:
        return vk::ImageAspectFlagBits::eDepth;
    case vk::Format::eD16UnormS8Uint:
    case vk::Format::eD24UnormS8Uint:
    case vk::Format::eD32SfloatS8Uint:
        return vk::ImageAspectFlagBits::eDepth |
               vk::ImageAspectFlagBits::eStencil;
    default:
        return vk::ImageAspectFlagBits::eColor;
    }
}

vk::ImageView VulkanImage::create_view() {
    vk::ImageViewCreateInfo info;
    info.image = *m_image;
    // XXX: Support other image types
    info.viewType = vk::ImageViewType::e2D;
    info.format = m_format;
    info.subresourceRange.aspectMask = all_aspects(m_format);
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    m_view = device()->createImageView(info, nullptr);
    return **m_view;
}
