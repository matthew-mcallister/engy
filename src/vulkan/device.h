#ifndef VULKAN_INSTANCE_H_INCLUDED
#define VULKAN_INSTANCE_H_INCLUDED

#include <span>

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "vulkan/debug.h"

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

    bool m_debug;

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

    bool debug() const { return m_debug; }
    const vk::raii::PhysicalDevice &physical_device() const {
        return m_physical_device;
    }
    vk::raii::Queue &graphics_queue() { return m_graphics_queue; }

    vk::raii::Semaphore
    create_semaphore(vk::SemaphoreType type = vk::SemaphoreType::eBinary) const;

    template<typename T>
    void set_name(const T &object, const char *name) const {
        vk::DebugUtilsObjectNameInfoEXT name_info;
        name_info.objectType = object_type<T>::value;
        name_info.objectHandle = *reinterpret_cast<const uint64_t *>(&*object);
        name_info.pObjectName = name;
        m_device.setDebugUtilsObjectNameEXT(name_info);
    }
};

class VulkanSwapchain {
    /// TODO: Should be shared_ptr when multithreading
    int m_width;
    int m_height;
    const VulkanDevice &m_device;
    vk::raii::SwapchainKHR m_swapchain;
    std::vector<vk::Image> m_images;
    std::vector<vk::raii::ImageView> m_image_views;
    vk::raii::Semaphore m_image_available_semaphore;
    uint32_t m_acquired_image = 0xffffffff;

public:
    VulkanSwapchain(const VulkanDevice &device,
                    vk::raii::SwapchainKHR swapchain,
                    std::vector<vk::Image> images,
                    std::vector<vk::raii::ImageView> image_views)
        : m_device{device}, m_swapchain(std::move(swapchain)),
          m_images{std::move(images)}, m_image_views{std::move(image_views)},
          m_image_available_semaphore{device.create_semaphore()} {}

    static auto create(const VulkanDevice &device,
                       vk::SwapchainKHR old_swapchain) -> VulkanSwapchain;

    vk::raii::SwapchainKHR &operator*() { return m_swapchain; }
    const vk::raii::SwapchainKHR &operator*() const { return m_swapchain; }
    vk::raii::SwapchainKHR *operator->() { return &m_swapchain; }
    const vk::raii::SwapchainKHR *operator->() const { return &m_swapchain; }

    int width() const { return m_width; }
    int height() const { return m_height; }

    vk::raii::Semaphore &image_acquire_semaphore() {
        return m_image_available_semaphore;
    }

    vk::Image &current_image() {
        assert(m_acquired_image != 0xffffffff);
        return m_images[m_acquired_image];
    }
    vk::raii::ImageView &current_image_view() {
        assert(m_acquired_image != 0xffffffff);
        return m_image_views[m_acquired_image];
    }
    std::span<vk::Image> images() { return m_images; }
    uint32_t current_image_index() const { return m_acquired_image; }
    vk::Format image_format() const {
        return m_device.m_swapchain_settings.format;
    };

    void acquire_next_image(uint64_t timeout);
    void present(vk::raii::Queue &queue,
                 std::span<const vk::Semaphore> wait_semaphores);
};

#endif
