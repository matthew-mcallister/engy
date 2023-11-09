#ifndef VULKAN_MEMORY_H_INCLUDED
#define VULKAN_MEMORY_H_INCLUDED

#include <memory>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

#include "vulkan/device.h"

class VulkanAllocation;
class VulkanBuffer;

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

    static VulkanBuffer
    create_buffer(std::shared_ptr<VulkanAllocator> allocator,
                  const vk::BufferCreateInfo &buffer_create_info,
                  const VmaAllocationCreateInfo &allocation_create_info);

    VulkanDevice &device() { return m_device; }
    const VulkanDevice &device() const { return m_device; }
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

    vk::DeviceSize size() const { return m_info.size; }
    void *data() const { return m_info.pMappedData; }
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
    VulkanBuffer(const VulkanBuffer &other) = delete;
    VulkanBuffer(VulkanBuffer &&other)
        : VulkanBuffer(std::move(other.m_allocator), other.m_allocation,
                       other.m_info, std::move(other.m_buffer)) {
        other.m_allocation = 0;
    }
    const vk::Buffer &operator*() const { return *m_buffer; }
};

// TODO: Subbuffers

#endif