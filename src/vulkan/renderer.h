#ifndef VULKAN_RENDERER_H_INCLUDED
#define VULKAN_RENDERER_H_INCLUDED

#include <vk_mem_alloc.h>

#include "asset.h"
#include "vulkan/device.h"

class StagingBuffer;

struct PerFrame {
    vk::raii::Semaphore end_of_frame_semaphore;
    vk::raii::CommandPool command_pool;
    vk::raii::CommandBuffer command_buffer;
    uint64_t frame_in_flight = 0;

    static PerFrame create(int index, VulkanDevice &device,
                           VulkanSwapchain &swapchain);
};

class VulkanRenderer {
    VulkanDevice m_device;
    VulkanSwapchain m_swapchain;
    VmaAllocator m_allocator = 0;

    std::vector<PerFrame> m_per_frame;
    vk::raii::Semaphore m_present_semaphore;

    std::vector<vk::raii::ShaderModule> m_shaders;
    std::vector<vk::raii::PipelineLayout> m_pipeline_layouts;
    std::vector<vk::raii::Pipeline> m_graphics_pipelines;

    uint64_t m_frame = 0;

    PerFrame &per_frame() { return m_per_frame[m_frame % m_per_frame.size()]; }

    vk::raii::ShaderModule &create_shader_module(std::span<const char> bytes);
    vk::raii::PipelineLayout &create_pipeline_layout();

    friend class StagingBuffer;

public:
    VulkanRenderer(VulkanDevice device, VulkanSwapchain swapchain);
    ~VulkanRenderer();

    VulkanDevice &device() { return m_device; }
    VulkanSwapchain &swapchain() { return m_swapchain; }

    void flush_frame();
    void begin_rendering();
    void render();
    void end_rendering();
    void acquire_image();
    void present();

    vk::raii::Pipeline &create_graphics_pipeline(AssetApi &assets);
};

#endif
