#ifndef TEXTURE_MAP_H_INCLUDED
#define TEXTURE_MAP_H_INCLUDED

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "asset.h"
#include "image.h"
#include "vulkan/device.h"
#include "vulkan/memory.h"
#include "vulkan/staging.h"

struct TextureMapEntry {
    uint32_t id;
    std::string path;
    VulkanImage image;
    vk::raii::ImageView view;
    uint64_t upload_batch;

    TextureMapEntry(uint32_t id, std::string path, VulkanImage image,
                    vk::raii::ImageView view)
        : id{id}, path{std::move(path)}, image{std::move(image)},
          view{std::move(view)} {}
};

const size_t TEXTURE_MAP_MAX_ENTRIES = 4096;

class TextureMap;

/// @brief Large bindless descriptor set from which combined image
/// sampler descriptors are allocated. Not to be confused with the
/// descriptor buffer extensions.
class ImageDescriptorHeap {
    VulkanDevice &m_device;
    vk::raii::DescriptorSetLayout m_layout;
    vk::raii::DescriptorPool m_pool;
    vk::raii::DescriptorSet m_set;
    uint32_t m_index = 0;

    friend class TextureMap;

    ImageDescriptorHeap(VulkanDevice &device,
                        vk::raii::DescriptorSetLayout layout,
                        vk::raii::DescriptorPool pool,
                        vk::raii::DescriptorSet uniforms)
        : m_device{device}, m_layout{std::move(layout)},
          m_pool{std::move(pool)}, m_set{std::move(uniforms)} {}

public:
    static ImageDescriptorHeap create(VulkanDevice &device);

    const vk::DescriptorSetLayout &layout() const { return *m_layout; }
    const vk::DescriptorSet &descriptor_set() const { return *m_set; }

    /// @brief Adds an image to the heap and returns the offset of the
    /// descriptor for use in shaders.
    uint32_t add(const vk::raii::ImageView &view,
                 const vk::raii::Sampler &sampler);
};

class TextureMap {
    AssetApi &m_assets;
    VulkanDevice &m_device;
    std::shared_ptr<VulkanAllocator> m_allocator;
    StagingBuffer &m_staging;
    ImageDescriptorHeap m_descriptor_heap;
    vk::raii::Sampler m_sampler;
    std::vector<TextureMapEntry> m_entries;
    std::unordered_map<std::string, uint32_t> m_entry_map;

    static vk::raii::Sampler create_sampler(VulkanDevice &device);

public:
    TextureMap(AssetApi &assets, std::shared_ptr<VulkanAllocator> allocator,
               StagingBuffer &staging)
        : m_assets{assets}, m_device{allocator->device()},
          m_allocator{std::move(allocator)}, m_staging{staging},
          m_descriptor_heap{ImageDescriptorHeap::create(m_device)},
          m_sampler{TextureMap::create_sampler(m_device)} {}

    const ImageDescriptorHeap &heap() const { return m_descriptor_heap; }

    uint32_t get(const std::string &path);
};

#endif
