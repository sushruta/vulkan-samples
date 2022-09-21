#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "container.h"
#include "helper.h"

namespace scg {
    void createBuffer(scg::sDevice& s_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(sDevice& s_device, sCommand& s_command, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(scg::sDevice& s_device, scg::sCommand& s_command, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    
    void transitionImageLayout(scg::sDevice& s_device, scg::sCommand& s_command, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    VkCommandBuffer beginSingleTimeCommands(scg::sDevice& s_device, scg::sCommand& s_command);
    void endSingleTimeCommands(scg::sDevice& s_device, scg::sCommand& s_command, VkCommandBuffer commandBuffer);

    void createUniformBuffers(sInstance& s_inst, sDevice& s_device, sUniformBuffer& s_ubuf);
    void createIndexBuffer(sDevice& s_device, sCommand& s_command, sGeometry& s_geom);
    void createVertexBuffer(sDevice& s_device, sCommand& s_command, sGeometry& s_geom);
}

void scg::createBuffer(scg::sDevice& s_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(s_device.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(s_device.device, buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = scg::findMemoryType(s_device.physicalDevice, memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(s_device.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(s_device.device, buffer, bufferMemory, 0);
}

    void scg::copyBuffer(sDevice& s_device, sCommand& s_command, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(s_device, s_command);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(s_device, s_command, commandBuffer);
    }

void scg::transitionImageLayout(scg::sDevice& s_device, scg::sCommand& s_command, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(s_device, s_command);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

    endSingleTimeCommands(s_device, s_command, commandBuffer);
}

void scg::copyBufferToImage(scg::sDevice& s_device, scg::sCommand& s_command, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = scg::beginSingleTimeCommands(s_device, s_command);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    scg::endSingleTimeCommands(s_device, s_command, commandBuffer);
}

VkCommandBuffer scg::beginSingleTimeCommands(scg::sDevice& s_device, scg::sCommand& s_command) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = s_command.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(s_device.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void scg::endSingleTimeCommands(scg::sDevice& s_device, scg::sCommand& s_command, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(s_device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(s_device.graphicsQueue);

    vkFreeCommandBuffers(s_device.device, s_command.commandPool, 1, &commandBuffer);
}

void scg::createVertexBuffer(sDevice& s_device, sCommand& s_command, sGeometry& s_geom) {
    VkDeviceSize bufferSize = sizeof(s_geom.vertices[0]) * s_geom.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    scg::createBuffer(s_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(s_device.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, s_geom.vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(s_device.device, stagingBufferMemory);

    scg::createBuffer(s_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s_geom.vertexBuffer, s_geom.vertexBufferMemory);

    copyBuffer(s_device, s_command, stagingBuffer, s_geom.vertexBuffer, bufferSize);

    vkDestroyBuffer(s_device.device, stagingBuffer, nullptr);
    vkFreeMemory(s_device.device, stagingBufferMemory, nullptr);
}

void scg::createIndexBuffer(sDevice& s_device, sCommand& s_command, sGeometry& s_geom) {
    VkDeviceSize bufferSize = sizeof(s_geom.indices[0]) * s_geom.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    scg::createBuffer(s_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(s_device.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, s_geom.indices.data(), (size_t) bufferSize);
    vkUnmapMemory(s_device.device, stagingBufferMemory);

    scg::createBuffer(s_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s_geom.indexBuffer, s_geom.indexBufferMemory);

    copyBuffer(s_device, s_command, stagingBuffer, s_geom.indexBuffer, bufferSize);

    vkDestroyBuffer(s_device.device, stagingBuffer, nullptr);
    vkFreeMemory(s_device.device, stagingBufferMemory, nullptr);
}

void scg::createUniformBuffers(sInstance& s_inst, sDevice& s_device, sUniformBuffer& s_ubuf) {
    VkDeviceSize bufferSize = sizeof(scg::UniformBufferObject);

    s_ubuf.uniformBuffers.resize(s_inst.maxFramesInFlight);
    s_ubuf.uniformBuffersMemory.resize(s_inst.maxFramesInFlight);

    for (size_t i = 0; i < s_inst.maxFramesInFlight; i++) {
        createBuffer(s_device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, s_ubuf.uniformBuffers[i], s_ubuf.uniformBuffersMemory[i]);
    }
}
