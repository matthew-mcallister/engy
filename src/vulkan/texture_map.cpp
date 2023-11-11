#include "vulkan/texture_map.h"

#include <cassert>
#include <format>
#include <optional>

#include <vulkan/vulkan_raii.hpp>

ImageDescriptorHeap ImageDescriptorHeap::create(VulkanDevice &device) {
    vk::DescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    binding.descriptorCount = 0x1'0000;
    binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
    auto flags = vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                 vk::DescriptorBindingFlagBits::ePartiallyBound |
                 vk::DescriptorBindingFlagBits::eUpdateAfterBind |
                 vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
    vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags;
    binding_flags.setBindingFlags(flags);
    vk::DescriptorSetLayoutCreateInfo layout_info;
    layout_info.pNext = &binding_flags;
    layout_info.flags =
        vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
    layout_info.setBindings(binding);
    auto set_layout = device->createDescriptorSetLayout(layout_info);
    device.set_name(*set_layout, "TextureMap.m_layout");

    vk::DescriptorPoolSize pool_size;
    pool_size.type = vk::DescriptorType::eCombinedImageSampler;
    pool_size.descriptorCount = 8192;
    vk::DescriptorPoolCreateInfo pool_info;
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                      vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
    pool_info.maxSets = 1;
    pool_info.setPoolSizes(pool_size);
    auto pool = device->createDescriptorPool(pool_info);
    device.set_name(*pool, "TextureMap.m_pool");

    uint32_t desc_count = TEXTURE_MAP_MAX_ENTRIES;
    vk::DescriptorSetVariableDescriptorCountAllocateInfo var_info;
    var_info.setDescriptorCounts(desc_count);
    vk::DescriptorSetAllocateInfo alloc_info;
    alloc_info.pNext = &var_info;
    alloc_info.descriptorPool = *pool;
    alloc_info.setSetLayouts(*set_layout);
    auto sets = device->allocateDescriptorSets(alloc_info);
    device.set_name(*sets[0], "TextureMap.m_set");

    ImageDescriptorHeap heap{device, std::move(set_layout), std::move(pool),
                             std::move(sets[0])};
    return heap;
}

uint32_t ImageDescriptorHeap::add(const vk::raii::ImageView &view,
                                  const vk::raii::Sampler &sampler) {
    auto index = m_index;
    m_index++;

    vk::DescriptorImageInfo info;
    info.sampler = *sampler;
    info.imageView = *view;
    info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::WriteDescriptorSet write;
    write.dstSet = *m_set;
    write.dstBinding = 0;
    write.dstArrayElement = index;
    write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    write.descriptorCount = 1;
    write.pImageInfo = &info;
    m_device->updateDescriptorSets(write, nullptr);

    return index;
}

vk::raii::Sampler TextureMap::create_sampler(VulkanDevice &device) {
    const auto properties = device.physical_device().getProperties2();
    const auto max_anisotropy =
        properties.properties.limits.maxSamplerAnisotropy;

    vk::SamplerCreateInfo info;
    info.magFilter = vk::Filter::eLinear;
    info.minFilter = vk::Filter::eLinear;
    info.mipmapMode = vk::SamplerMipmapMode::eLinear;
    info.addressModeU = vk::SamplerAddressMode::eRepeat;
    info.addressModeV = vk::SamplerAddressMode::eRepeat;
    info.addressModeW = vk::SamplerAddressMode::eRepeat;
    info.anisotropyEnable = 1;
    info.maxAnisotropy = max_anisotropy;
    info.maxLod = 30.0;
    auto sampler = device->createSampler(info, nullptr);
    device.set_name(*sampler, "TextureMap.m_sampler");
    return sampler;
}

uint32_t TextureMap::get(const std::string &path) {
    auto result = m_entry_map.find(path);
    if (result != m_entry_map.end()) {
        return m_entries[result->second].id;
    }

    const auto src = m_assets.load_image(path);

    // Step 1: Create and allocate image
    vk::ImageCreateInfo info;
    info.imageType = vk::ImageType::e2D;
    info.format = format_to_vk(src.format());
    info.extent = vk::Extent3D(src.width(), src.height(), 1);
    // TODO: mipmapping
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = vk::SampleCountFlagBits::e1;
    info.tiling = vk::ImageTiling::eOptimal;
    info.usage = vk::ImageUsageFlagBits::eSampled |
                 vk::ImageUsageFlagBits::eTransferSrc |
                 vk::ImageUsageFlagBits::eTransferDst;
    info.initialLayout = vk::ImageLayout::eUndefined;

    VmaAllocationCreateInfo alloc_info;
    memset(&alloc_info, 0, sizeof(VmaAllocationCreateInfo));
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    auto image = VulkanAllocator::create_image(m_allocator, info, alloc_info);

    // Step 2: Create image view
    vk::ImageViewCreateInfo view_info;
    view_info.image = *image;
    view_info.viewType = vk::ImageViewType::e2D;
    view_info.format = info.format;
    view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    auto view = m_device->createImageView(view_info, nullptr);

    // Step 3: Write the descriptor
    const auto id = m_descriptor_heap.add(view, m_sampler);
    m_device.set_name(*image, std::format("TextureMap[{}].image", id).c_str());
    m_device.set_name(*view, std::format("TextureMap[{}].view", id).c_str());

    // Step 4: Add the entry
    TextureMapEntry entry{id, path, std::move(image), std::move(view)};
    entry.upload_batch = m_staging.stage_image(src, entry.image);
    m_entries.push_back(std::move(entry));
    m_entry_map.insert({path, id});

    // The image can now be read in shaders by indexing the samplers
    // uniform array at this index
    return id;
}
