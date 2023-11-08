#ifndef VULKAN_RENDERER_H_INCLUDED
#define VULKAN_RENDERER_H_INCLUDED

#include <vk_mem_alloc.h>

#include "asset.h"
#include "vulkan/device.h"
#include "vulkan/memory.h"
#include "vulkan/staging.h"

struct PerFrame {
    vk::raii::Semaphore end_of_frame_semaphore;
    vk::raii::CommandPool command_pool;
    vk::raii::CommandBuffer command_buffer;

    VulkanBuffer uniforms;

    uint64_t frame_in_flight = 0;

    static PerFrame create(int index, VulkanDevice &device,
                           VulkanSwapchain &swapchain,
                           std::shared_ptr<VulkanAllocator> allocator);
};

class VulkanRenderer {
    VulkanDevice m_device;
    VulkanSwapchain m_swapchain;
    std::shared_ptr<VulkanAllocator> m_allocator;
    StagingBuffer m_staging;

    std::vector<PerFrame> m_per_frame;
    vk::raii::Semaphore m_present_semaphore;

    std::vector<vk::raii::ShaderModule> m_shaders;
    std::vector<vk::raii::DescriptorSetLayout> m_set_layouts;
    std::vector<vk::raii::PipelineLayout> m_pipeline_layouts;
    std::vector<vk::raii::Pipeline> m_graphics_pipelines;

    uint64_t m_frame = 0;

    PerFrame &per_frame() { return m_per_frame[m_frame % m_per_frame.size()]; }

    vk::raii::DescriptorSetLayout &create_set_layout();
    vk::raii::ShaderModule &create_shader_module(std::span<const char> bytes);
    vk::raii::PipelineLayout &create_pipeline_layout();

    friend class StagingBuffer;

public:
    VulkanRenderer(VulkanDevice device, VulkanSwapchain swapchain);
    ~VulkanRenderer();

    VulkanDevice &device() { return m_device; }
    VulkanSwapchain &swapchain() { return m_swapchain; }

    // XXX: Move these methods to PerFrame class
    void flush_frame();
    void begin_rendering();
    void update_and_bind_uniforms();
    void render();
    void end_rendering();
    void acquire_image();
    void present();

    vk::raii::Pipeline &create_graphics_pipeline(AssetApi &assets);
};

#endif
