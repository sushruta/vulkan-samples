#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "container.h"
//#include "device.h"

namespace scg {
    void createDescriptorSetLayout(scg::sDevice& s_device, scg::sDescriptor& s_descriptor);
    void createDescriptorSets(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sDescriptor& s_descriptor, scg::sUniformBuffer& s_ubuf, scg::sTexture& s_texture);
    void createDescriptorPool(sInstance& s_inst, sDevice& s_device, sDescriptor& s_descriptor);
}

    void scg::createDescriptorPool(sInstance& s_inst, sDevice& s_device, sDescriptor& s_descriptor) {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(s_inst.maxFramesInFlight);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(s_inst.maxFramesInFlight);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(s_inst.maxFramesInFlight);

        if (vkCreateDescriptorPool(s_device.device, &poolInfo, nullptr, &(s_descriptor.descriptorPool)) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

void scg::createDescriptorSetLayout(scg::sDevice& s_device, scg::sDescriptor& s_descriptor) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(s_device.device, &layoutInfo, nullptr, &(s_descriptor.descriptorSetLayout)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

    void scg::createDescriptorSets(scg::sInstance& s_inst, scg::sDevice& s_device, scg::sDescriptor& s_descriptor, scg::sUniformBuffer& s_ubuf, scg::sTexture& s_texture) {
        std::vector<VkDescriptorSetLayout> layouts(s_inst.maxFramesInFlight, s_descriptor.descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = s_descriptor.descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(s_inst.maxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        std::cout << "init size of layouts and allocInfo{}" << std::endl;

        s_descriptor.descriptorSets.resize(s_inst.maxFramesInFlight);
        if (vkAllocateDescriptorSets(s_device.device, &allocInfo, s_descriptor.descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        std::cout << "resized and allocated memory for s_descriptor.descriptorSets" << std::endl;

        for (size_t i = 0; i < s_inst.maxFramesInFlight; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = s_ubuf.uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(scg::UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = s_texture.textureImageView;
            imageInfo.sampler = s_texture.textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = s_descriptor.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = s_descriptor.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(s_device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
