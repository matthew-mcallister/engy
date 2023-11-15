#ifndef RENDER_FULLSCREEN_PASS_H_INCLUDED
#define RENDER_FULLSCREEN_PASS_H_INCLUDED

#include <vulkan/vulkan_raii.hpp>

#include "vulkan/renderer.h"

// Base class for fullscreen passes with depth/g-buffer (as opposed to
// post-process pass which only has color/hdr).
class FullscreenPass {
    VulkanRenderer &m_renderer;
    std::string m_fragment_shader;
    vk::raii::Pipeline m_pipeline;

public:
    FullscreenPass(VulkanRenderer &renderer, std::string fragment_shader,
                   const vk::raii::PipelineLayout &layout);

    virtual void bind(vk::raii::CommandBuffer &commands);
    void draw(vk::raii::CommandBuffer &commands);
};

class SkyPass : public FullscreenPass {
    vk::raii::PipelineLayout m_layout;

    SkyPass(VulkanRenderer &renderer, vk::raii::PipelineLayout layout);

public:
    SkyPass(VulkanRenderer &renderer);
};

#endif