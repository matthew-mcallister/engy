#ifndef VULKAN_RENDERER_H_INCLUDED
#define VULKAN_RENDERER_H_INCLUDED

#include "vulkan/device.h"

class VulkanRenderer {
    VulkanDevice m_device;
    VulkanSwapchain m_swapchain;

public:
    VulkanRenderer(VulkanDevice device, VulkanSwapchain swapchain)
        : m_device{std::move(device)}, m_swapchain{std::move(swapchain)} {}
};

#endif
