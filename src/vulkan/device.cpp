#include <algorithm>
#include <format>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "exceptions.h"
#include "vulkan/device.h"

void load_vulkan_library() {
    if (SDL_Vulkan_LoadLibrary(nullptr)) {
        throw SystemException("Failed to load Vulkan");
    }
}

SwapchainSettings validate_device(const vk::raii::SurfaceKHR &surface,
                                  const vk::raii::PhysicalDevice &device) {
    const auto id = device.getProperties().deviceID;
    if (!device.getSurfaceSupportKHR(0, *surface)) {
        throw SystemException(
            std::format("Device {} does not support window presentation", id));
    }

    const auto surface_formats = device.getSurfaceFormatsKHR(*surface);
    const vk::SurfaceFormatKHR *selected_format = nullptr;
    for (const auto &format : surface_formats) {
        if (format.colorSpace != vk::ColorSpaceKHR::eSrgbNonlinear) {
            continue;
        }
        if (format.format == vk::Format::eB8G8R8A8Srgb) {
            selected_format = &format;
            break;
        } else if (format.format == vk::Format::eB8G8R8A8Unorm) {
            selected_format = &format;
        }
    }
    if (!selected_format) {
        throw SystemException(std::format(
            "Device {} does not support required pixel format", id));
    }
    const auto color_space = selected_format->colorSpace;
    const auto format = selected_format->format;

    const auto present_modes = device.getSurfacePresentModesKHR(*surface);
    auto present_mode = vk::PresentModeKHR::eImmediate;
    for (const auto &mode : present_modes) {
        if (mode == vk::PresentModeKHR::eFifo) {
            present_mode = mode;
            break;
        } else if (mode == vk::PresentModeKHR::eFifoRelaxed) {
            present_mode = mode;
        }
    }

    return SwapchainSettings{
        format,
        color_space,
        present_mode,
    };
}

VulkanDevice VulkanDevice::create(SDL_Window *window, uint32_t device_id,
                                  bool debug) {
    std::vector<const char *> requested_layers;
    std::vector<const char *> required_extensions;

    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    required_extensions.resize(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, &required_extensions[0]);

    if (debug) {
        requested_layers.push_back("VK_LAYER_KHRONOS_validation");
        required_extensions.push_back("VK_EXT_debug_utils");
    }

    vk::raii::Context context;

    const auto layers = context.enumerateInstanceLayerProperties();
    auto it = requested_layers.begin();
    while (it != requested_layers.end()) {
        for (const auto &layer : layers) {
            if (std::strcmp(*it, layer.layerName) == 0) {
                goto next_iter;
            }
        }
        requested_layers.erase(it);
    next_iter:
        it++;
    }

    vk::ApplicationInfo app_info;
    app_info.pApplicationName = "demo app";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "engy";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_3;
    vk::InstanceCreateInfo inst_info;
    inst_info.pApplicationInfo = &app_info;
    inst_info.setPEnabledExtensionNames(required_extensions);
    inst_info.setPEnabledLayerNames(requested_layers);
    vk::raii::Instance instance(context, inst_info);

    VkSurfaceKHR vk_surface;
    if (SDL_Vulkan_CreateSurface(window, *instance, &vk_surface) != SDL_TRUE) {
        throw SystemException("Failed to create surface");
    }
    vk::raii::SurfaceKHR surface{instance, vk_surface, nullptr};

    vk::raii::PhysicalDevices phys_devices{instance};
    vk::raii::PhysicalDevice *selected = nullptr;
    SwapchainSettings sw_settings;
    if (device_id) {
        for (vk::raii::PhysicalDevice &pdev : phys_devices) {
            const auto properties = pdev.getProperties();
            if (properties.deviceID == device_id) {
                sw_settings = validate_device(surface, pdev);
                selected = &pdev;
                break;
            }
        }
        throw SystemException("No such graphics device: " + device_id);
    } else if (!phys_devices.empty()) {
        for (auto &pdev : phys_devices) {
            try {
                sw_settings = validate_device(surface, pdev);
                selected = &pdev;
                break;
            } catch (const std::exception &e) {
                continue;
            }
        }
    }
    if (!selected) {
        throw SystemException("No suitable graphics device found");
    }
    vk::raii::PhysicalDevice pdev = std::move(*selected);

    const auto queue_families = pdev.getQueueFamilyProperties();
    if (!(queue_families[0].queueFlags & vk::QueueFlagBits::eGraphics)) {
        // Queue 0 is almost always a graphics/compute/transfer queue
        throw SystemException("No graphics queue");
    }

    required_extensions.clear();
    required_extensions.push_back("VK_KHR_swapchain");
    required_extensions.push_back("VK_KHR_push_descriptor");

    // Configure graphics queue
    vk::DeviceQueueCreateInfo queue_info;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    float priority = 1.0;
    queue_info.setQueuePriorities(priority);

    // TODO: Check that selected features are compatible with device.

    vk::PhysicalDeviceSynchronization2Features synchronization2_features;
    synchronization2_features.synchronization2 = 1;

    vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features;
    dynamic_rendering_features.pNext = &synchronization2_features;
    dynamic_rendering_features.dynamicRendering = 1;

    vk::PhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features;
    timeline_semaphore_features.pNext = &dynamic_rendering_features;
    timeline_semaphore_features.timelineSemaphore = 1;

    vk::PhysicalDeviceDescriptorIndexingFeatures desc_indexing_features;
    desc_indexing_features.pNext = &timeline_semaphore_features;
    desc_indexing_features.shaderSampledImageArrayNonUniformIndexing = 1;
    desc_indexing_features.descriptorBindingSampledImageUpdateAfterBind = 1;
    desc_indexing_features.descriptorBindingUpdateUnusedWhilePending = 1;
    desc_indexing_features.descriptorBindingPartiallyBound = 1;
    desc_indexing_features.descriptorBindingVariableDescriptorCount = 1;

    vk::PhysicalDeviceFeatures2 features;
    features.pNext = &desc_indexing_features;
    features.features.samplerAnisotropy = 1;

    vk::DeviceCreateInfo dev_info;
    dev_info.setQueueCreateInfos(queue_info);
    dev_info.pNext = &features;
    dev_info.setPEnabledExtensionNames(required_extensions);
    vk::raii::Device vk_device{pdev, dev_info};

    auto graphics_queue = vk_device.getQueue(0, 0);
    VulkanDevice device{window,
                        std::move(context),
                        std::move(instance),
                        std::move(pdev),
                        std::move(vk_device),
                        std::move(graphics_queue),
                        std::move(surface),
                        sw_settings};
    return device;
}

vk::raii::Semaphore
VulkanDevice::create_semaphore(vk::SemaphoreType type) const {
    vk::SemaphoreTypeCreateInfo type_info;
    type_info.semaphoreType = type;
    vk::SemaphoreCreateInfo info;
    info.pNext = &type_info;
    return m_device.createSemaphore(info, nullptr);
}

VulkanSwapchain VulkanSwapchain::create(const VulkanDevice &device,
                                        vk::SwapchainKHR old_swapchain) {
    const auto settings = device.m_swapchain_settings;
    int w, h;
    SDL_Vulkan_GetDrawableSize(device.m_window, &w, &h);
    const auto capabilities =
        device.physical_device().getSurfaceCapabilitiesKHR(*device.m_surface);
    vk::SwapchainCreateInfoKHR sw_info;
    sw_info.setSurface(*device.m_surface);
    sw_info.minImageCount = capabilities.minImageCount + 1;
    sw_info.imageFormat = settings.format;
    sw_info.imageColorSpace = settings.color_space;
    sw_info.imageExtent = vk::Extent2D{w, h};
    sw_info.imageArrayLayers = 1;
    sw_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    sw_info.imageSharingMode = vk::SharingMode::eExclusive;
    const uint32_t queue_family_index = 0;
    sw_info.setQueueFamilyIndices(queue_family_index);
    sw_info.presentMode = settings.present_mode;
    sw_info.oldSwapchain = old_swapchain;
    auto swapchain = device->createSwapchainKHR(sw_info, nullptr);

    auto images = swapchain.getImages();
    std::vector<vk::raii::ImageView> image_views;
    for (const auto &image : images) {
        vk::ImageViewCreateInfo info;
        info.image = image;
        info.viewType = vk::ImageViewType::e2D;
        info.format = settings.format;
        info.subresourceRange = vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        auto view = device->createImageView(info, nullptr);
        image_views.push_back(std::move(view));
    }

    auto sw = VulkanSwapchain{device, std::move(swapchain), std::move(images),
                              std::move(image_views)};
    sw.m_width = w;
    sw.m_height = h;
    return sw;
}

void VulkanSwapchain::acquire_next_image(uint64_t timeout) {
    auto result = m_swapchain.acquireNextImage(
        timeout, *m_image_available_semaphore, nullptr);
    if (result.first == vk::Result::eTimeout) {
        throw TimeoutException("Timed out waiting for swapchain image");
    }
    m_acquired_image = result.second;
}

void VulkanSwapchain::present(vk::raii::Queue &queue,
                              std::span<const vk::Semaphore> wait_semaphores) {
    vk::PresentInfoKHR info;
    info.setWaitSemaphores(wait_semaphores);
    info.setSwapchains(*m_swapchain);
    info.setImageIndices(m_acquired_image);
    info.pResults = nullptr;
    auto result = queue.presentKHR(info);
    if (result == vk::Result::eErrorDeviceLost) {
        throw DeviceLostException("Graphics device lost");
    }
}
