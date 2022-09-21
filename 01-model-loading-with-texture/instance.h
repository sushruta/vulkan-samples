#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <stdexcept>

#include "helper.h"
#include "container.h"

namespace scg {
    void createInstance(scg::sInstance& s_inst);
    void createSurface(scg::sInstance& s_inst);
}

void scg::createInstance(scg::sInstance& s_inst) {
    if (s_inst.enableValidationLayers && !scg::checkValidationLayerSupport(s_inst.validationLayers)) {
        throw std::runtime_error("validation layers requested, but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = s_inst.appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = scg::getRequiredExtensions(s_inst.enableValidationLayers);

#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    std::cout << ">> Vulkan device extensions being loaded -" << std::endl;
    for (auto& extension : extensions) {
        std::cout << extension << std::endl;
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
#ifdef NDEBUG
    createInfo.enabledLayerCount = static_cast<uint32_t>(s_inst.validationLayers.size());
    createInfo.ppEnabledLayerNames = s_inst.validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#else
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &(s_inst.instance)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void scg::createSurface(scg::sInstance& s_inst) {
    if (glfwCreateWindowSurface(s_inst.instance, s_inst.window, nullptr, &s_inst.surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}
