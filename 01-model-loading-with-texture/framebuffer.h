#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <stdexcept>

#include "container.h"

namespace scg {
    void createFramebuffers(scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sRenderPass& s_rpass, scg::sDepth& s_depth, scg::sFramebuffer& s_fbuf);
}

void scg::createFramebuffers(scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sRenderPass& s_rpass, scg::sDepth& s_depth, scg::sFramebuffer& s_fbuf) {
    s_fbuf.swapchainFramebuffers.resize(s_swapchain.swapchainImageViews.size());

    for (size_t i = 0; i < s_swapchain.swapchainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            s_swapchain.swapchainImageViews[i],
            s_depth.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = s_rpass.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = s_swapchain.swapchainExtent.width;
        framebufferInfo.height = s_swapchain.swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(s_device.device, &framebufferInfo, nullptr, &(s_fbuf.swapchainFramebuffers[i])) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
