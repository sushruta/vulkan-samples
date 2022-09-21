#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "container.h"
#include "format.h"
#include "swapchain.h"

namespace scg {
    void createDepthResources(scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sDepth& s_depth);
}

void scg::createDepthResources(scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sDepth& s_depth) {
    VkFormat depthFormat = scg::findDepthFormat(s_device);

    scg::createImage(s_device, s_swapchain.swapchainExtent.width, s_swapchain.swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s_depth.depthImage, s_depth.depthImageMemory);
    s_depth.depthImageView = scg::createImageView(s_device, s_depth.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

