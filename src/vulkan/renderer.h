#ifndef VULKAN_RENDERER_H_INCLUDED
#define VULKAN_RENDERER_H_INCLUDED

#include "vulkan/device.h"

struct PerFrame {
    vk::raii::Semaphore end_of_frame_semaphore;
    vk::raii::CommandPool command_pool;
    vk::raii::CommandBuffer command_buffer;
    uint64_t frame_in_flight = 0;

    static PerFrame create(VulkanDevice &device, VulkanSwapchain &swapchain);
};

class VulkanRenderer {
    VulkanDevice m_device;
    VulkanSwapchain m_swapchain;

    std::vector<PerFrame> m_per_frame;
    vk::raii::Semaphore m_present_semaphore;

    uint64_t m_frame = 0;

    PerFrame &per_frame() { return m_per_frame[m_frame % m_per_frame.size()]; }

public:
    VulkanRenderer(VulkanDevice device, VulkanSwapchain swapchain);
    ~VulkanRenderer();

    VulkanDevice &device() { return m_device; }
    VulkanSwapchain &swapchain() { return m_swapchain; }

    void flush_frame();
    void begin_rendering();
    void end_rendering();
    void acquire_image();
    void present();
};

#endif
