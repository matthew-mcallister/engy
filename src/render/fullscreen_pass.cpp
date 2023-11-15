#include <span>

#include "render/fullscreen_pass.h"

vk::raii::Pipeline
create_fullscreen_pipeline(VulkanRenderer &renderer, std::string shader_path,
                           const vk::raii::PipelineLayout &layout) {
    auto &vertex_shader =
        renderer.create_shader_module("shaders/fullscreen.vertex.spv");
    vk::PipelineShaderStageCreateInfo vertex_stage;
    vertex_stage.stage = vk::ShaderStageFlagBits::eVertex;
    vertex_stage.module = *vertex_shader;
    vertex_stage.pName = "main";
    auto &fragment_shader = renderer.create_shader_module(shader_path);
    vk::PipelineShaderStageCreateInfo fragment_stage;
    fragment_stage.stage = vk::ShaderStageFlagBits::eFragment;
    fragment_stage.module = *fragment_shader;
    fragment_stage.pName = "main";
    std::array<vk::PipelineShaderStageCreateInfo, 2> stages = {vertex_stage,
                                                               fragment_stage};

    vk::PipelineVertexInputStateCreateInfo vertex_input;

    vk::PipelineInputAssemblyStateCreateInfo input_assembly;
    input_assembly.topology = vk::PrimitiveTopology::eTriangleStrip;

    int w = renderer.swapchain().width(), h = renderer.swapchain().height();
    vk::Viewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = w;
    viewport.height = h;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    vk::Rect2D scissor = {{0, 0}, {w, h}};
    vk::PipelineViewportStateCreateInfo viewport_state;
    viewport_state.setViewports(viewport);
    viewport_state.setScissors(scissor);

    vk::PipelineRasterizationStateCreateInfo raster_state;
    raster_state.polygonMode = vk::PolygonMode::eFill;
    raster_state.cullMode = vk::CullModeFlagBits::eNone;
    raster_state.lineWidth = 1.0;

    vk::PipelineMultisampleStateCreateInfo multisample_state;
    multisample_state.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depth_state;
    depth_state.depthTestEnable = 1;
    depth_state.depthWriteEnable = 0;
    depth_state.depthCompareOp = vk::CompareOp::eGreaterOrEqual;

    vk::PipelineColorBlendAttachmentState attachment;
    attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    vk::PipelineColorBlendStateCreateInfo color_blend;
    color_blend.setAttachments(attachment);

    vk::PipelineRenderingCreateInfo rendering_info;
    vk::Format color_format = renderer.swapchain().image_format();
    rendering_info.setColorAttachmentFormats(color_format);
    rendering_info.depthAttachmentFormat = vk::Format::eD32Sfloat;

    vk::GraphicsPipelineCreateInfo info;
    info.pNext = &rendering_info;
    info.setStages(stages);
    info.pVertexInputState = &vertex_input;
    info.pInputAssemblyState = &input_assembly;
    info.pViewportState = &viewport_state;
    info.pRasterizationState = &raster_state;
    info.pMultisampleState = &multisample_state;
    info.pDepthStencilState = &depth_state;
    info.pColorBlendState = &color_blend;
    info.layout = *layout;

    auto pipeline =
        renderer.device()->createGraphicsPipeline(nullptr, info, nullptr);
    return pipeline;
}

FullscreenPass::FullscreenPass(VulkanRenderer &renderer,
                               std::string fragment_shader,
                               const vk::raii::PipelineLayout &layout)
    : m_renderer{renderer}, m_fragment_shader{std::move(fragment_shader)},
      m_pipeline{
          create_fullscreen_pipeline(m_renderer, m_fragment_shader, layout)} {}

void FullscreenPass::bind(vk::raii::CommandBuffer &commands) {
    commands.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
}

void FullscreenPass::draw(vk::raii::CommandBuffer &commands) {
    bind(commands);
    commands.draw(4, 1, 0, 0);
}

vk::raii::PipelineLayout create_sky_pipeline_layout(VulkanRenderer &renderer) {
    vk::PipelineLayoutCreateInfo info;
    info.setSetLayouts(*renderer.uniform_set_layout());
    auto layout = renderer.device()->createPipelineLayout(info, nullptr);
    return layout;
}

SkyPass::SkyPass(VulkanRenderer &renderer, vk::raii::PipelineLayout layout)
    : FullscreenPass(renderer, "shaders/sky.fragment.spv", layout),
      m_layout{std::move(layout)} {}

SkyPass::SkyPass(VulkanRenderer &renderer)
    : SkyPass(renderer, create_sky_pipeline_layout(renderer)) {}
