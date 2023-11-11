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
