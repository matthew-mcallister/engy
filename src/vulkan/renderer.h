#ifndef VULKAN_RENDERER_H_INCLUDED
#define VULKAN_RENDERER_H_INCLUDED

#include <unordered_map>
#include <vector>

#include <vk_mem_alloc.h>

#include "asset.h"
#include "math/matrix.h"
#include "vulkan/device.h"
#include "vulkan/memory.h"
#include "vulkan/mesh.h"
#include "vulkan/staging.h"
#include "vulkan/texture_map.h"

class Chunk;

struct ViewUniforms {
    Vector4 viewport;
    Matrix4 projection;
    Matrix4 view;
    Matrix4 view_inverse;
};

struct Uniforms {
    ViewUniforms view_uniforms;
    Matrix4 instance[512];
};

struct PerFrame {
    vk::raii::Semaphore end_of_frame_semaphore;
    vk::raii::CommandPool command_pool;
    vk::raii::CommandBuffer command_buffer;

    VulkanImage depth_buffer;
    VulkanBuffer uniforms;

    uint64_t frame_in_flight = 0;

    static PerFrame create(int index, VulkanDevice &device,
                           const VulkanSwapchain &swapchain,
                           std::shared_ptr<VulkanAllocator> allocator);
};

class VulkanRenderer {
    AssetApi &m_assets;
    VulkanDevice m_device;
    VulkanSwapchain m_swapchain;
    std::shared_ptr<VulkanAllocator> m_allocator;
    StagingBuffer m_staging;
    TextureMap m_texture_map;

    std::vector<PerFrame> m_per_frame;
    vk::raii::Semaphore m_present_semaphore;

    std::unordered_map<std::string, vk::raii::ShaderModule> m_shaders;
    std::vector<vk::raii::DescriptorSetLayout> m_set_layouts;
    std::vector<vk::raii::PipelineLayout> m_pipeline_layouts;
    std::vector<vk::raii::Pipeline> m_graphics_pipelines;

    uint64_t m_frame = 0;
    uint32_t m_instance = 0;

    PerFrame &per_frame() { return m_per_frame[m_frame % m_per_frame.size()]; }

    vk::raii::DescriptorSetLayout &create_set_layout();
    vk::raii::PipelineLayout &create_pipeline_layout();
    void bind_textures();
    void bind_uniforms();

    friend class StagingBuffer;

public:
    VulkanRenderer(AssetApi &assets, VulkanDevice device,
                   VulkanSwapchain swapchain);

    VulkanDevice &device() { return m_device; }
    VulkanSwapchain &swapchain() { return m_swapchain; }
    const std::shared_ptr<VulkanAllocator> &allocator() const {
        return m_allocator;
    }
    StagingBuffer &staging() { return m_staging; }
    TextureMap &textures() { return m_texture_map; }

    Mesh create_mesh(std::span<const char> vertex_data,
                     std::span<const uint32_t> index_data);
    uint32_t load_texture(const std::string &path) {
        return m_texture_map.get(path);
    }

    // XXX: Move these methods to PerFrame class
    void flush_frame();
    void begin_rendering();
    void update_uniforms(const ViewUniforms &view);
    void begin_rendering_meshes();
    void render_mesh(const Mesh &mesh, Matrix4 instance);
    void render_chunk(const Chunk &chunk);
    void end_rendering();
    void acquire_image();
    void present();
    void wait_idle();

    vk::raii::ShaderModule &create_shader_module(const std::string &name);
    vk::raii::Pipeline &create_graphics_pipeline();

    // XXX: Get rid of this crap through refactoring
    vk::raii::DescriptorSetLayout &uniform_set_layout() {
        return m_set_layouts[0];
    }
    vk::raii::CommandBuffer &commands() { return per_frame().command_buffer; }
};

#endif
