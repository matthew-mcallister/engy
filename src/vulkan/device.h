#ifndef VULKAN_INSTANCE_H_INCLUDED
#define VULKAN_INSTANCE_H_INCLUDED

#include <span>

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class VulkanSwapchain;

void load_vulkan_library();

struct SwapchainSettings {
    vk::Format format;
    vk::ColorSpaceKHR color_space;
    vk::PresentModeKHR present_mode;
};

class VulkanDevice {
    SDL_Window *m_window;
    vk::raii::Instance m_instance;
    vk::raii::PhysicalDevice m_physical_device;
    vk::raii::Device m_device;
    vk::raii::Queue m_graphics_queue;
    vk::raii::SurfaceKHR m_surface;
    SwapchainSettings m_swapchain_settings;

    friend class VulkanSwapchain;

public:
    VulkanDevice(SDL_Window *window, vk::raii::Instance instance,
                 vk::raii::PhysicalDevice physical_device,
                 vk::raii::Device device, vk::raii::Queue graphics_queue,
                 vk::raii::SurfaceKHR surface,
                 SwapchainSettings swapchain_settings)
        : m_window{window}, m_instance{std::move(instance)},
          m_physical_device(std::move(physical_device)),
          m_device{std::move(device)},
          m_graphics_queue{std::move(graphics_queue)},
          m_surface{std::move(surface)},
          m_swapchain_settings(swapchain_settings) {}

    static auto create(SDL_Window *window, uint32_t device_id, bool debug)
        -> VulkanDevice;

    vk::raii::Device &operator*() { return m_device; }
    const vk::raii::Device &operator*() const { return m_device; }
    vk::raii::Device *operator->() { return &m_device; }
    const vk::raii::Device *operator->() const { return &m_device; }

    const vk::raii::PhysicalDevice &physical_device() const {
        return m_physical_device;
    }
    vk::raii::Queue &graphics_queue() { return m_graphics_queue; }

    vk::raii::Semaphore create_binary_semaphore() const;
};

class VulkanSwapchain {
    /// TODO: Should be shared_ptr when multithreading
    const VulkanDevice &m_device;
    vk::raii::SwapchainKHR m_swapchain;
    vk::raii::Semaphore m_image_available_semaphore;
    uint32_t m_acquired_image = 0xffffffff;

public:
    VulkanSwapchain(const VulkanDevice &device,
                    vk::raii::SwapchainKHR swapchain)
        : m_device{device}, m_swapchain(std::move(swapchain)),
          m_image_available_semaphore{device.create_binary_semaphore()} {}

    static auto create(const VulkanDevice &device,
                       vk::SwapchainKHR old_swapchain) -> VulkanSwapchain;

    vk::raii::SwapchainKHR &operator*() { return m_swapchain; }
    const vk::raii::SwapchainKHR &operator*() const { return m_swapchain; }
    vk::raii::SwapchainKHR *operator->() { return &m_swapchain; }
    const vk::raii::SwapchainKHR *operator->() const { return &m_swapchain; }

    vk::raii::Semaphore &image_available_semaphore() {
        return m_image_available_semaphore;
    }

    void acquire_next_image(uint64_t timeout);
    void present(vk::raii::Queue &queue,
                 std::span<const vk::Semaphore> wait_semaphores);
};

#endif