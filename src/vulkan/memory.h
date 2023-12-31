#ifndef VULKAN_MEMORY_H_INCLUDED
#define VULKAN_MEMORY_H_INCLUDED

#include <memory>
#include <optional>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

#include "vulkan/device.h"

class VulkanAllocation;
class VulkanBuffer;
class VulkanImage;

// TODO: Buffer suballocation. VMA is quite stupid to not have that as a
// feature.
class VulkanAllocator {
    VulkanDevice &m_device;
    VmaAllocator m_allocator;

    friend class VulkanAllocation;
    friend class VulkanBuffer;

public:
    VulkanAllocator(VulkanDevice &device,
                    const VmaAllocatorCreateInfo &create_info);
    ~VulkanAllocator();

    VulkanDevice &device() { return m_device; }
    const VulkanDevice &device() const { return m_device; }

    static VulkanBuffer
    create_buffer(std::shared_ptr<VulkanAllocator> allocator,
                  const vk::BufferCreateInfo &buffer_create_info,
                  const VmaAllocationCreateInfo &allocation_create_info);
    static VulkanImage
    create_image(std::shared_ptr<VulkanAllocator> allocator,
                 const vk::ImageCreateInfo &image_create_info,
                 const VmaAllocationCreateInfo &allocation_create_info);
};

class VulkanAllocation {
protected:
    // XXX: Maybe this should just be a reference instead of shared_ptr
    // as we don't strictly need reference counting.
    std::shared_ptr<VulkanAllocator> m_allocator;
    VmaAllocation m_allocation;
    VmaAllocationInfo m_info;

    friend class VulkanAllocator;

    VulkanAllocation(std::shared_ptr<VulkanAllocator> allocator,
                     VmaAllocation allocation, VmaAllocationInfo info)
        : m_allocator{std::move(allocator)}, m_allocation{allocation},
          m_info{info} {}

public:
    VulkanAllocation(const VulkanAllocation &other) = delete;
    VulkanAllocation(VulkanAllocation &&other)
        : VulkanAllocation(std::move(other.m_allocator), other.m_allocation,
                           other.m_info) {
        other.m_allocation = 0;
    }
    ~VulkanAllocation() {
        if (m_allocation) {
            vmaFreeMemory(m_allocator->m_allocator, m_allocation);
        }
    }

    VulkanAllocation &operator=(VulkanAllocation &&other) {
        m_allocator = std::move(other.m_allocator);
        m_allocation = other.m_allocation;
        other.m_allocation = 0;
        m_info = other.m_info;
        return *this;
    }
    VulkanAllocation &operator=(const VulkanAllocation &other) = delete;

    VulkanDevice &device() { return m_allocator->device(); }
    const VulkanDevice &device() const { return m_allocator->device(); }
    vk::DeviceSize size() const { return m_info.size; }
    void *data() { return m_info.pMappedData; }
    const void *data() const { return m_info.pMappedData; }
};

class VulkanBuffer : public VulkanAllocation {
    vk::raii::Buffer m_buffer;

    friend class VulkanAllocator;

    VulkanBuffer(std::shared_ptr<VulkanAllocator> allocator,
                 VmaAllocation allocation, VmaAllocationInfo info,
                 vk::raii::Buffer buffer)
        : VulkanAllocation(allocator, allocation, info),
          m_buffer{std::move(buffer)} {}

public:
    VulkanBuffer(VulkanBuffer &&other) = default;
    VulkanBuffer(const VulkanBuffer &other) = delete;
    VulkanBuffer &operator=(VulkanBuffer &&other) = default;
    VulkanBuffer &operator=(const VulkanBuffer &other) = delete;
    const vk::Buffer &operator*() const { return *m_buffer; }
};

// TODO: Subbuffers

class VulkanImage : VulkanAllocation {
    vk::raii::Image m_image;
    std::optional<vk::raii::ImageView> m_view;
    vk::Extent3D m_extent;
    uint32_t m_mip_levels = 1;
    uint32_t m_array_layers = 1;
    // TODO: Probably want to restrict this to a smaller selection of choices
    vk::Format m_format;

    friend class VulkanAllocator;

    VulkanImage(std::shared_ptr<VulkanAllocator> allocator,
                VmaAllocation allocation, VmaAllocationInfo alloc_info,
                vk::raii::Image image, vk::Extent3D extent, uint32_t mip_levels,
                uint32_t array_layers, vk::Format format)
        : VulkanAllocation(allocator, allocation, alloc_info),
          m_image{std::move(image)}, m_extent{extent}, m_mip_levels{mip_levels},
          m_array_layers{array_layers}, m_format{format} {}

public:
    VulkanImage(VulkanImage &&other) = default;
    VulkanImage(const VulkanImage &other) = delete;
    VulkanImage &operator=(VulkanImage &&other) = default;
    VulkanImage &operator=(const VulkanImage &other) = delete;

    const vk::Image &operator*() const { return *m_image; }

    uint32_t width() const { return m_extent.width; }
    uint32_t height() const { return m_extent.height; }
    uint32_t depth() const { return m_extent.depth; }
    vk::Extent3D extent() const { return m_extent; }
    uint32_t mip_levels() const { return m_mip_levels; }
    uint32_t array_layers() const { return m_array_layers; }
    vk::Format format() const { return m_format; }
    /// @brief If an image view has been created, returns it. Else
    /// returns VK_NULL_HANDLE;
    vk::ImageView view() const { return m_view ? **m_view : vk::ImageView{}; }

    /// @brief Creates an image view if it does not already exist and
    /// returns the image view.
    vk::ImageView create_view();
};

#endif