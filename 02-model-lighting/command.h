#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "container.h"

namespace scg {
    void createCommandPool(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sCommand& s_command);
    void createCommandBuffers(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sCommand& s_command);
}

void scg::createCommandPool(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sCommand& s_command) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(s_device.physicalDevice, s_inst.surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(s_device.device, &poolInfo, nullptr, &(s_command.commandPool)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void scg::createCommandBuffers(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sCommand& s_command) {
    s_command.commandBuffers.resize(s_inst.maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = s_command.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) s_command.commandBuffers.size();

    if (vkAllocateCommandBuffers(s_device.device, &allocInfo, s_command.commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
