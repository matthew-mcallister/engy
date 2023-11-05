#include <array>

#include "exceptions.h"
#include "vulkan/renderer.h"

PerFrame PerFrame::create(VulkanDevice &device, VulkanSwapchain &swapchain) {
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

    return {std::move(semaphore), std::move(pool), std::move(buffers[0])};
}

VulkanRenderer::VulkanRenderer(VulkanDevice device, VulkanSwapchain swapchain)
    : m_device{std::move(device)}, m_swapchain{std::move(swapchain)},
      m_present_semaphore(m_device.create_semaphore()) {
    for (int i = 0; i < 2; i++) {
        m_per_frame.push_back(PerFrame::create(m_device, m_swapchain));
    }
}

VulkanRenderer::~VulkanRenderer() {
    m_device->waitIdle();
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
