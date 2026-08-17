#include "vk_descriptor_allocator.h"

void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (PoolSizeRatio ratio : poolRatios)
	{
		poolSizes.push_back(
			VkDescriptorPoolSize
			{
				ratio.type,
				uint32_t(ratio.ratio * maxSets)
			});
	}

	VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.flags = 0;
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool);
}

void DescriptorAllocator::ClearDescriptors(VkDevice device)
{
	vkResetDescriptorPool(device, m_pool, 0);
}

void DescriptorAllocator::DestroyPool(VkDevice device)
{
	vkDestroyDescriptorPool(device, m_pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = m_pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet ds;
	VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));

	return ds;
}
