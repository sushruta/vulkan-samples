#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <iostream>

#include "helper.h"
#include "container.h"
#include "instance.h"
#include "device.h"
#include "swapchain.h"
#include "renderpass.h"
#include "descriptor.h"
#include "graphicspipeline.h"
#include "command.h"
#include "depth.h"
#include "framebuffer.h"
#include "texture.h"
#include "synchronization.h"
#include "buffer.h"

class VulkanApplication {
public:
    VulkanApplication() = delete;
    VulkanApplication(uint32_t width, uint32_t height, std::string appName);
    void run();
private:
    void cleanup();
    void initVulkan();
    void initWindow();
    void mainLoop();
    void drawFrame();
    void cleanupSwapchain();
    void updateUniformBuffer(scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sUniformBuffer& s_ubuf, uint32_t currentImage);
    void recreateSwapchain();
    void recordCommandBuffer(uint32_t imageIndex);

    scg::sInstance s_inst;
    scg::sDevice s_device;
    scg::sSwapchain s_swapchain;
    scg::sRenderPass s_rpass;
    scg::sDescriptor s_descriptor;
    scg::sGraphicsPipeline s_gpipeline;
    scg::sCommand s_command;
    scg::sDepth s_depth;
    scg::sFramebuffer s_fbuf;
    scg::sTexture s_texture;
    scg::sSynch s_synch;
    scg::sUniformBuffer s_ubuf;
    scg::sGeometry s_geom;

    bool framebufferResized{false};
    bool isAppleDevice{false};
    int currentFrame{0};

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
};

VulkanApplication::VulkanApplication(uint32_t width, uint32_t height, std::string appName) {
    s_inst.width = width;
    s_inst.height = height;
    s_inst.appName = appName;

#ifdef NDEBUG
    s_inst.enableValidationLayers = true;
#else
    s_inst.enableValidationLayers = false;
#endif

#ifdef __APPLE__
    isAppleDevice = true;
#endif
}

void VulkanApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanApplication::mainLoop() {
    while (!glfwWindowShouldClose(s_inst.window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(s_device.device);
}

void VulkanApplication::initVulkan() {
    scg::createInstance(s_inst);
    if (s_inst.enableValidationLayers) {
        scg::setupDebugMessenger(s_inst.instance, s_inst.debugMessenger);
    }
    scg::createSurface(s_inst);
    scg::createDevice(s_inst, s_device);
    scg::createSwapchain(s_inst, s_device, s_swapchain);
    scg::createImageViews(s_device, s_swapchain);
    scg::createRenderPass(s_device, s_swapchain, s_rpass);
    scg::createDescriptorSetLayout(s_device, s_descriptor);
    scg::createGraphicsPipeline(s_device, s_descriptor, s_rpass, s_gpipeline);
    scg::createCommandPool(s_inst, s_device, s_command);
    scg::createDepthResources(s_device, s_swapchain, s_depth);
    scg::createFramebuffers(s_device, s_swapchain, s_rpass, s_depth, s_fbuf);
    scg::createTextureImage(s_inst, s_device, s_command, s_texture);
    scg::createTextureImageView(s_device, s_texture);
    scg::createTextureSampler(s_device, s_texture);
    std::cout << "completed creating command buffers" << std::endl;
    scg::loadModel(s_inst, s_geom);
    std::cout << "completed loading the obj model" << std::endl;
    scg::createVertexBuffer(s_device, s_command, s_geom);
    scg::createIndexBuffer(s_device, s_command, s_geom);
    scg::createUniformBuffers(s_inst, s_device, s_ubuf);
    scg::createDescriptorPool(s_inst, s_device, s_descriptor);
    scg::createDescriptorSets(s_inst, s_device, s_descriptor, s_ubuf, s_texture);
    std::cout << "completed creating descriptor sets" << std::endl;
    scg::createCommandBuffers(s_inst, s_device, s_command);
    scg::createSynchObjects(s_inst, s_device, s_synch);
    std::cout << "completed synch objects" << std::endl;
}

// no resizing for now
void VulkanApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    s_inst.window = glfwCreateWindow(s_inst.width, s_inst.height, s_inst.appName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(s_inst.window, this);

    glfwSetFramebufferSizeCallback(s_inst.window, framebufferResizeCallback);
}

void VulkanApplication::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(s_inst.window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(s_inst.window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(s_device.device);

    cleanupSwapchain();

    scg::createSwapchain(s_inst, s_device, s_swapchain);
    scg::createImageViews(s_device, s_swapchain);
    scg::createDepthResources(s_device, s_swapchain, s_depth);
    scg::createFramebuffers(s_device, s_swapchain, s_rpass, s_depth, s_fbuf);
}

void VulkanApplication::updateUniformBuffer(scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sUniformBuffer& s_ubuf, uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    scg::UniformBufferObject ubo{};
    
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = ubo.model * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.model = ubo.model * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f) * 10.0f);
    ubo.model = ubo.model * glm::translate(glm::mat4(1.0f), glm::vec3(-0.0019, 0.1396, -0.0108) * -1.0f);
    
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), s_swapchain.swapchainExtent.width / (float) s_swapchain.swapchainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(s_device.device, s_ubuf.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(s_device.device, s_ubuf.uniformBuffersMemory[currentImage]);
}

// (scg::sDevice& s_device, scg::sSwapchain& s_swapchain, scg::sCommand& s_command, scg::sUniformBuffer& s_ubuf, scg::sSynchronization& s_synch)
void VulkanApplication::drawFrame() {
    vkWaitForFences(s_device.device, 1, &(s_synch.inFlightFences[currentFrame]), VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(s_device.device, s_swapchain.swapchain, UINT64_MAX, s_synch.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // get this method
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(s_device, s_swapchain, s_ubuf, currentFrame);

    vkResetFences(s_device.device, 1, &(s_synch.inFlightFences[currentFrame]));

    vkResetCommandBuffer(s_command.commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {s_synch.imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(s_command.commandBuffers[currentFrame]);

    VkSemaphore signalSemaphores[] = {(s_synch.renderFinishedSemaphores[currentFrame])};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(s_device.graphicsQueue, 1, &submitInfo, s_synch.inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {s_swapchain.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(s_device.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        // get this method
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % s_inst.maxFramesInFlight;
}

// s_swapchain, s_rpass, s_command, s_fbuf
void VulkanApplication::recordCommandBuffer(uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(s_command.commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = s_rpass.renderPass;
    renderPassInfo.framebuffer = s_fbuf.swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = s_swapchain.swapchainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(s_command.commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(s_command.commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, s_gpipeline.graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) s_swapchain.swapchainExtent.width;
    viewport.height = (float) s_swapchain.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(s_command.commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = s_swapchain.swapchainExtent;
    vkCmdSetScissor(s_command.commandBuffers[currentFrame], 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {s_geom.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(s_command.commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(s_command.commandBuffers[currentFrame], s_geom.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(s_command.commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, s_gpipeline.pipelineLayout, 0, 1, &(s_descriptor.descriptorSets[currentFrame]), 0, nullptr);

    vkCmdDrawIndexed(s_command.commandBuffers[currentFrame], static_cast<uint32_t>(s_geom.indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(s_command.commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(s_command.commandBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanApplication::cleanupSwapchain() {
    vkDestroyImageView(s_device.device, s_depth.depthImageView, nullptr);
    vkDestroyImage(s_device.device, s_depth.depthImage, nullptr);
    vkFreeMemory(s_device.device, s_depth.depthImageMemory, nullptr);

    for (auto framebuffer : s_fbuf.swapchainFramebuffers) {
        vkDestroyFramebuffer(s_device.device, framebuffer, nullptr);
    }

    for (auto imageView : s_swapchain.swapchainImageViews) {
        vkDestroyImageView(s_device.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(s_device.device, s_swapchain.swapchain, nullptr);
}

void VulkanApplication::cleanup() {
    cleanupSwapchain();

    vkDestroyPipeline(s_device.device, s_gpipeline.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(s_device.device, s_gpipeline.pipelineLayout, nullptr);
    vkDestroyRenderPass(s_device.device, s_rpass.renderPass, nullptr);

    for (size_t i = 0; i < s_inst.maxFramesInFlight; i++) {
            vkDestroyBuffer(s_device.device, s_ubuf.uniformBuffers[i], nullptr);
            vkFreeMemory(s_device.device, s_ubuf.uniformBuffersMemory[i], nullptr);
        }

    vkDestroyDescriptorPool(s_device.device, s_descriptor.descriptorPool, nullptr);

    vkDestroySampler(s_device.device, s_texture.textureSampler, nullptr);
    vkDestroyImageView(s_device.device, s_texture.textureImageView, nullptr);
    vkDestroyImage(s_device.device, s_texture.textureImage, nullptr);
    vkFreeMemory(s_device.device, s_texture.textureImageMemory, nullptr);

    vkDestroyDescriptorSetLayout(s_device.device, s_descriptor.descriptorSetLayout, nullptr);

    vkDestroyBuffer(s_device.device, s_geom.indexBuffer, nullptr);
    vkFreeMemory(s_device.device, s_geom.indexBufferMemory, nullptr);

    vkDestroyBuffer(s_device.device, s_geom.vertexBuffer, nullptr);
    vkFreeMemory(s_device.device, s_geom.vertexBufferMemory, nullptr);

    for (size_t i = 0; i < s_inst.maxFramesInFlight; i++) {
        vkDestroySemaphore(s_device.device, s_synch.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(s_device.device, s_synch.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(s_device.device, s_synch.inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(s_device.device, s_command.commandPool, nullptr);

    vkDestroyDevice(s_device.device, nullptr);

    if (s_inst.enableValidationLayers) {
        scg::DestroyDebugUtilsMessengerEXT(s_inst.instance, s_inst.debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(s_inst.instance, s_inst.surface, nullptr);
    vkDestroyInstance(s_inst.instance, nullptr);

    glfwDestroyWindow(s_inst.window);

    glfwTerminate();
}

