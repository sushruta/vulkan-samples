#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>

#include "container.h"

namespace scg {
    void createDevice(scg::sInstance& s_inst, scg::sDevice& s_device);
    void pickPhysicalDevice(scg::sInstance& s_inst, VkPhysicalDevice& physicalDevice);
    void createLogicalDevice(scg::sInstance& s_inst, scg::sDevice& s_device);
}

void scg::createDevice(scg::sInstance& s_inst, scg::sDevice& s_device) {
    scg::pickPhysicalDevice(s_inst, s_device.physicalDevice);
    scg::createLogicalDevice(s_inst, s_device);
}

void scg::pickPhysicalDevice(sInstance& s_inst, VkPhysicalDevice& physicalDevice) {
    uint32_t deviceCount{0};
    vkEnumeratePhysicalDevices(s_inst.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(s_inst.instance, &deviceCount, devices.data());

    for (auto& pdevice : devices) {
        if (scg::isDeviceSuitable(pdevice, s_inst.surface, s_inst.deviceExtensions)) {
            physicalDevice = pdevice;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void scg::createLogicalDevice(sInstance& s_inst, sDevice& s_device) {
    scg::QueueFamilyIndices indices = findQueueFamilies(s_device.physicalDevice, s_inst.surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(s_inst.deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_inst.deviceExtensions.data();

    if (s_inst.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(s_inst.validationLayers.size());
        createInfo.ppEnabledLayerNames = s_inst.validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(s_device.physicalDevice, &createInfo, nullptr, &(s_device.device)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(s_device.device, indices.graphicsFamily.value(), 0, &(s_device.graphicsQueue));
    vkGetDeviceQueue(s_device.device, indices.presentFamily.value(), 0, &(s_device.presentQueue));
}
