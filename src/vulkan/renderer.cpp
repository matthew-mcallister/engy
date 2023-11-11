#include <array>
#include <format>
#include <memory>

#include <vk_mem_alloc.h>

#include "asset.h"
#include "exceptions.h"
#include "math/vector.h"
#include "vulkan/memory.h"
#include "vulkan/renderer.h"

struct Uniforms {
    Vector4 color;
};

VulkanBuffer create_uniforms(std::shared_ptr<VulkanAllocator> allocator) {
    vk::BufferCreateInfo buffer_info;
    buffer_info.size = sizeof(Uniforms);
    buffer_info.usage = vk::BufferUsageFlagBits::eUniformBuffer;

    VmaAllocationCreateInfo alloc_info;
    memset(&alloc_info, 0, sizeof(VmaAllocationCreateInfo));
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    return VulkanAllocator::create_buffer(std::move(allocator), buffer_info,
                                          alloc_info);
}

PerFrame PerFrame::create(int index, VulkanDevice &device,
                          VulkanSwapchain &swapchain,
                          std::shared_ptr<VulkanAllocator> allocator) {
    auto semaphore = device.create_semaphore(vk::SemaphoreType::eTimeline);

    vk::CommandPoolCreateInfo cmd_info;
    cmd_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
    cmd_info.queueFamilyIndex = 0;
    vk::raii::CommandPool pool{*device, cmd_info, nullptr};

    vk::CommandBufferAllocateInfo alloc_info;
    alloc_info.commandPool = *pool;
    alloc_info.level = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandBufferCount = 1;
    auto buffers = device->allocateCommandBuffers(alloc_info);
    auto &buffer = buffers[0];

    auto uniforms = create_uniforms(std::move(allocator));

    if (device.debug()) {
        std::string name;
        name = std::format("PerFrame[{}].end_of_frame_semaphore", index);
        device.set_name(*semaphore, name.c_str());
        name = std::format("PerFrame[{}].command_pool", index);
        device.set_name(*pool, name.c_str());
        name = std::format("PerFrame[{}].command_buffer", index);
        device.set_name(*buffer, name.c_str());
        name = std::format("PerFrame[{}].uniforms", index);
        device.set_name(*uniforms, name.c_str());
    }

    return {std::move(semaphore), std::move(pool), std::move(buffer),
            std::move(uniforms)};
}

vk::raii::ShaderModule &
VulkanRenderer::create_shader_module(std::span<const char> bytes) {
    assert(bytes.size() % 4 == 0);
    const std::span<const uint32_t> code2{(const uint32_t *)&bytes[0],
                                          bytes.size() / 4};
    vk::ShaderModuleCreateInfo info;
    info.setCode(code2);
    auto module = m_device->createShaderModule(info, nullptr);
    m_shaders.push_back(std::move(module));
    return m_shaders[m_shaders.size() - 1];
}

vk::raii::DescriptorSetLayout &VulkanRenderer::create_set_layout() {
    vk::DescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    binding.descriptorCount = 1;
    binding.stageFlags =
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    vk::DescriptorSetLayoutCreateInfo info;
    info.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;
    info.setBindings(binding);
    auto layout = m_device->createDescriptorSetLayout(info, nullptr);
    m_set_layouts.push_back(std::move(layout));
    return m_set_layouts[0];
}

vk::raii::PipelineLayout &VulkanRenderer::create_pipeline_layout() {
    vk::DescriptorSetLayout texmap_layout = m_texture_map.heap().layout();
    vk::DescriptorSetLayout uniform_layout = *create_set_layout();
    std::array<vk::DescriptorSetLayout, 2> layouts = {texmap_layout,
                                                      uniform_layout};
    vk::PipelineLayoutCreateInfo info;
    info.setSetLayouts(layouts);
    auto layout = m_device->createPipelineLayout(info, nullptr);
    m_device.set_name(*layout, "Renderer.m_pipeline_layouts[0]");
    m_pipeline_layouts.push_back(std::move(layout));
    return m_pipeline_layouts[m_pipeline_layouts.size() - 1];
}

vk::raii::Pipeline &VulkanRenderer::create_graphics_pipeline(AssetApi &assets) {
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    auto &vertex_shader =
        create_shader_module(assets.load_blob("shaders/triangle.vertex.spv"));
    vk::PipelineShaderStageCreateInfo vertex_stage;
    vertex_stage.stage = vk::ShaderStageFlagBits::eVertex;
    vertex_stage.module = *vertex_shader;
    vertex_stage.pName = "main";
    stages.push_back(vertex_stage);
    auto &fragment_shader =
        create_shader_module(assets.load_blob("shaders/triangle.fragment.spv"));
    vk::PipelineShaderStageCreateInfo fragment_stage;
    fragment_stage.stage = vk::ShaderStageFlagBits::eFragment;
    fragment_stage.module = *fragment_shader;
    fragment_stage.pName = "main";
    stages.push_back(fragment_stage);

    vk::VertexInputAttributeDescription position_attr;
    position_attr.location = 0;
    position_attr.binding = 0;
    position_attr.format = vk::Format::eR32G32B32Sfloat;
    position_attr.offset = 0;
    vk::VertexInputBindingDescription vertex_binding;
    vertex_binding.binding = 0;
    vertex_binding.stride = 3 * sizeof(float);
    vertex_binding.inputRate = vk::VertexInputRate::eVertex;
    vk::PipelineVertexInputStateCreateInfo vertex_input;
    vertex_input.setVertexAttributeDescriptions(position_attr);
    vertex_input.setVertexBindingDescriptions(vertex_binding);

    vk::PipelineInputAssemblyStateCreateInfo input_assembly;
    input_assembly.topology = vk::PrimitiveTopology::eTriangleList;

    int w = m_swapchain.width(), h = m_swapchain.height();
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
    raster_state.cullMode =
        vk::CullModeFlagBits::eNone; // FIXME: Backface culling
    raster_state.frontFace = vk::FrontFace::eCounterClockwise;
    raster_state.lineWidth = 1.0;

    vk::PipelineMultisampleStateCreateInfo multisample_state;
    multisample_state.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState attachment;
    attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    vk::PipelineColorBlendStateCreateInfo color_blend;
    color_blend.setAttachments(attachment);

    vk::PipelineRenderingCreateInfo rendering_info;
    vk::Format color_format = m_swapchain.image_format();
    rendering_info.setColorAttachmentFormats(color_format);

    const vk::PipelineLayout &layout = *create_pipeline_layout();

    vk::GraphicsPipelineCreateInfo info;
    info.pNext = &rendering_info;
    info.setStages(stages);
    info.pVertexInputState = &vertex_input;
    info.pInputAssemblyState = &input_assembly;
    info.pViewportState = &viewport_state;
    info.pRasterizationState = &raster_state;
    info.pMultisampleState = &multisample_state;
    info.pColorBlendState = &color_blend;
    info.layout = layout;

    auto pipeline = m_device->createGraphicsPipeline(nullptr, info, nullptr);
    m_device.set_name(*pipeline, "Renderer.m_graphics_pipelines[0]");
    m_graphics_pipelines.push_back(std::move(pipeline));
    return m_graphics_pipelines[m_graphics_pipelines.size() - 1];
}

std::shared_ptr<VulkanAllocator> create_allocator(VulkanDevice &device) {
    VmaVulkanFunctions functions = {};
    functions.vkGetInstanceProcAddr =
        device.context().getDispatcher()->vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr =
        device.physical_device().getDispatcher()->vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo info = {};
    info.vulkanApiVersion = VK_API_VERSION_1_3;
    info.instance = *device.instance();
    info.physicalDevice = *device.physical_device();
    info.device = **device;
    info.pVulkanFunctions = &functions;

    std::shared_ptr<VulkanAllocator> allocator{
        new VulkanAllocator(device, info)};
    return allocator;
}

VulkanRenderer::VulkanRenderer(AssetApi &assets, VulkanDevice device,
                               VulkanSwapchain swapchain)
    : m_assets{assets}, m_device{std::move(device)},
      m_swapchain{std::move(swapchain)},
      m_allocator{create_allocator(m_device)},
      m_staging{StagingBuffer::create(m_allocator, 0x200'0000)},
      m_texture_map{m_assets, m_allocator, m_staging},
      m_present_semaphore(m_device.create_semaphore()) {
    for (int i = 0; i < 2; i++) {
        m_per_frame.push_back(
            PerFrame::create(i, m_device, m_swapchain, m_allocator));
    }
}

Mesh VulkanRenderer::create_mesh(std::span<const char> vertex_data,
                                 std::span<const uint32_t> index_data) {
    return Mesh{m_allocator, m_staging, vertex_data, index_data};
}

void VulkanRenderer::flush_frame() {
    m_frame++;
    auto &frame = per_frame();
    if (frame.frame_in_flight > 0) {
        vk::SemaphoreWaitInfo info;
        info.setSemaphores(*frame.end_of_frame_semaphore);
        info.setValues(frame.frame_in_flight);
        auto result = m_device->waitSemaphores(info, 16'000'000);
        if (result == vk::Result::eTimeout) {
            // Roll back frame bump so frame can be retried
            m_frame--;
            throw TimeoutException("Timed out waiting for frame to finish");
        }
    }
    frame.frame_in_flight = m_frame;
}

void VulkanRenderer::begin_rendering() {
    auto &frame = per_frame();
    frame.command_pool.reset();
    auto &cmds = frame.command_buffer;

    vk::CommandBufferBeginInfo begin_info;
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmds.begin(begin_info);

    vk::ImageMemoryBarrier2 barrier;
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
    barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    barrier.oldLayout = vk::ImageLayout::eUndefined;
    barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.image = m_swapchain.current_image();
    barrier.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1,
    };
    vk::DependencyInfo dep;
    dep.setImageMemoryBarriers(barrier);
    cmds.pipelineBarrier2(dep);

    vk::RenderingAttachmentInfo att_info;
    att_info.imageView = *m_swapchain.current_image_view();
    att_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    att_info.loadOp = vk::AttachmentLoadOp::eClear;
    att_info.storeOp = vk::AttachmentStoreOp::eStore;
    att_info.clearValue.color.float32 = std::array{0.0f, 0.0f, 0.0f, 0.0f};
    vk::RenderingInfo info;
    info.renderArea =
        vk::Rect2D{{}, vk::Extent2D{m_swapchain.width(), m_swapchain.height()}};
    info.layerCount = 1;
    info.setColorAttachments(att_info);
    cmds.beginRendering(info);
}

void VulkanRenderer::bind_textures() {
    auto &frame = per_frame();
    auto &cmds = frame.command_buffer;
    const auto &set = m_texture_map.heap().descriptor_set();
    cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                            *m_pipeline_layouts[0], 0, set, nullptr);
}

void VulkanRenderer::update_and_bind_uniforms() {
    auto &frame = per_frame();
    auto &cmds = frame.command_buffer;
    ((Uniforms *)frame.uniforms.data())->color = Vector4(0, 1, 0, 1);
    vk::DescriptorBufferInfo buf_info;
    buf_info.buffer = *frame.uniforms;
    buf_info.offset = 0;
    buf_info.range = VK_WHOLE_SIZE;
    vk::WriteDescriptorSet write;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = vk::DescriptorType::eUniformBuffer;
    write.setBufferInfo(buf_info);
    cmds.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics,
                              *m_pipeline_layouts[0], 1, write);
}

void VulkanRenderer::begin_rendering_meshes() {
    auto &frame = per_frame();
    auto &cmds = frame.command_buffer;
    bind_textures();
    update_and_bind_uniforms();
    assert(m_graphics_pipelines.size() > 0);
    cmds.bindPipeline(vk::PipelineBindPoint::eGraphics,
                      *m_graphics_pipelines[0]);
}

void VulkanRenderer::render_mesh(const Mesh &mesh) {
    auto &frame = per_frame();
    auto &cmds = frame.command_buffer;
    mesh.bind(cmds);
    cmds.drawIndexed(mesh.size(), 1, 0, 0, 0);
}

void VulkanRenderer::end_rendering() {
    auto &frame = per_frame();
    auto &cmds = frame.command_buffer;
    cmds.endRendering();

    vk::ImageMemoryBarrier2 barrier;
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
    barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
    barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.image = m_swapchain.current_image();
    barrier.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1,
    };
    vk::DependencyInfo dep;
    dep.setImageMemoryBarriers(barrier);
    cmds.pipelineBarrier2(dep);

    cmds.end();

    vk::SemaphoreSubmitInfo wait_acquire;
    wait_acquire.semaphore = *m_swapchain.image_acquire_semaphore();
    wait_acquire.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    std::vector<vk::SemaphoreSubmitInfo> signal_infos;
    vk::SemaphoreSubmitInfo signal_end_of_frame;
    signal_end_of_frame.semaphore = *frame.end_of_frame_semaphore;
    signal_end_of_frame.stageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
    signal_end_of_frame.value = m_frame;
    signal_infos.push_back(signal_end_of_frame);
    vk::SemaphoreSubmitInfo signal_present;
    signal_present.semaphore = *m_present_semaphore;
    signal_present.stageMask =
        vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    signal_infos.push_back(signal_present);
    vk::CommandBufferSubmitInfo submit_cmds;
    submit_cmds.commandBuffer = *cmds;
    vk::SubmitInfo2 info;
    info.setWaitSemaphoreInfos(wait_acquire);
    info.setSignalSemaphoreInfos(signal_infos);
    info.setCommandBufferInfos(submit_cmds);
    m_device.graphics_queue().submit2(info, nullptr);
}

void VulkanRenderer::acquire_image() {
    m_swapchain.acquire_next_image(16'000'000);
}

void VulkanRenderer::present() {
    m_swapchain.present(m_device.graphics_queue(), {&*m_present_semaphore, 1});
}

void VulkanRenderer::wait_idle() {
    m_device->waitIdle();
}
