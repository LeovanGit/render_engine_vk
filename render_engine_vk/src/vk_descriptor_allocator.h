#pragma once

#include "types.h"

struct DescriptorAllocator
{
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);

	void ClearDescriptors(VkDevice device);

	void DestroyPool(VkDevice device);

	VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);

	VkDescriptorPool m_pool;
};
