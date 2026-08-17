#include "vk_descriptors.h"

void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
{
    VkDescriptorSetLayoutBinding newBind = {};
    newBind.binding = binding;
    newBind.descriptorCount = 1;
    newBind.descriptorType = type;

    m_bindings.push_back(newBind);
}

void DescriptorLayoutBuilder::Clear()
{
    m_bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(
    VkDevice device,
    VkShaderStageFlags shaderStages,
    void* pNext,
    VkDescriptorSetLayoutCreateFlags flags)
{
    for (auto& bind : m_bindings)
    {
        bind.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.pNext = pNext;
    info.pBindings = m_bindings.data();
    info.bindingCount = (uint32_t)m_bindings.size();
    info.flags = flags;

    VkDescriptorSetLayout set;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

    return set;
}
