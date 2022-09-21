#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>

#include "container.h"

namespace scg {
    void createSynchObjects(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sSynch& s_synch);
}

void scg::createSynchObjects(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sSynch& s_synch) {
    s_synch.imageAvailableSemaphores.resize(s_inst.maxFramesInFlight);
    s_synch.renderFinishedSemaphores.resize(s_inst.maxFramesInFlight);
    s_synch.inFlightFences.resize(s_inst.maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < s_inst.maxFramesInFlight; i++) {
        if (vkCreateSemaphore(s_device.device, &semaphoreInfo, nullptr, &(s_synch.imageAvailableSemaphores[i])) != VK_SUCCESS ||
                vkCreateSemaphore(s_device.device, &semaphoreInfo, nullptr, &(s_synch.renderFinishedSemaphores[i])) != VK_SUCCESS ||
                vkCreateFence(s_device.device, &fenceInfo, nullptr, &(s_synch.inFlightFences[i])) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

