#pragma once

#include "types.h"

struct DescriptorLayoutBuilder
{
    void AddBinding(uint32_t binding, VkDescriptorType type);

    void Clear();

    VkDescriptorSetLayout Build(
        VkDevice device,
        VkShaderStageFlags shaderStages,
        void* pNext = nullptr,
        VkDescriptorSetLayoutCreateFlags flags = 0);

    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};
