#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "helper.h"
#include "container.h"
#include "buffer.h"
#include "swapchain.h"

namespace scg {
    void createTextureImage(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sCommand& s_command, scg::sTexture& s_texture);
    void createTextureImageView(scg::sDevice& s_device, scg::sTexture& s_texture);
    void createTextureSampler(scg::sDevice& s_device, scg::sTexture& s_texture);
}

void scg::createTextureSampler(scg::sDevice& s_device, scg::sTexture& s_texture) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(s_device.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(s_device.device, &samplerInfo, nullptr, &(s_texture.textureSampler)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void scg::createTextureImageView(scg::sDevice& s_device, scg::sTexture& s_texture) {
    s_texture.textureImageView = scg::createImageView(s_device, s_texture.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void scg::createTextureImage(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sCommand& s_command, scg::sTexture& s_texture) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(s_inst.texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    scg::createBuffer(s_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    
    vkMapMemory(s_device.device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(s_device.device, stagingBufferMemory);
    
    stbi_image_free(pixels);
    scg::createImage(s_device, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s_texture.textureImage, s_texture.textureImageMemory);
    
    transitionImageLayout(s_device, s_command, s_texture.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    scg::copyBufferToImage(s_device, s_command, stagingBuffer, s_texture.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(s_device, s_command, s_texture.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(s_device.device, stagingBuffer, nullptr);
    vkFreeMemory(s_device.device, stagingBufferMemory, nullptr);
}
