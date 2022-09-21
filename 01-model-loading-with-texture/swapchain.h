#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "helper.h"
#include "container.h"
//#include "instance.h"
//#include "device.h"

namespace scg {
    void createSwapchain(sInstance& s_inst, sDevice& s_device, scg::sSwapchain& s_swapchain);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(sInstance& s_inst, const VkSurfaceCapabilitiesKHR& capabilities);
    VkImageView createImageView(scg::sDevice& s_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImageViews(scg::sDevice& s_device, scg::sSwapchain& s_swapchain);
}

void scg::createSwapchain(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sSwapchain& s_swapchain) {
    scg::SwapchainSupportDetails swapchainSupport = scg::querySwapchainSupport(s_device.physicalDevice, s_inst.surface);

    VkSurfaceFormatKHR surfaceFormat = scg::chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = scg::chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = scg::chooseSwapExtent(s_inst, swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = s_inst.surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = scg::findQueueFamilies(s_device.physicalDevice, s_inst.surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(s_device.device, &createInfo, nullptr, &(s_swapchain.swapchain)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(s_device.device, s_swapchain.swapchain, &imageCount, nullptr);
    s_swapchain.swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(s_device.device, s_swapchain.swapchain, &imageCount, s_swapchain.swapchainImages.data());

    s_swapchain.swapchainImageFormat = surfaceFormat.format;
    s_swapchain.swapchainExtent = extent;
}

VkSurfaceFormatKHR scg::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR scg::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D scg::chooseSwapExtent(scg::sInstance& s_inst, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(s_inst.window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkImageView scg::createImageView(scg::sDevice& s_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(s_device.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void scg::createImageViews(scg::sDevice& s_device, scg::sSwapchain& s_swapchain) {
    s_swapchain.swapchainImageViews.resize(s_swapchain.swapchainImages.size());

    for (uint32_t i = 0; i < s_swapchain.swapchainImages.size(); i++) {
        s_swapchain.swapchainImageViews[i] = createImageView(s_device, s_swapchain.swapchainImages[i], s_swapchain.swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}
