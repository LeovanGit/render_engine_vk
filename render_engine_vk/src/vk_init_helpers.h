#pragma once

#include "types.h"

namespace vk_init_helpers
{
	VkCommandPoolCreateInfo CommandPoolCreateInfo(
		uint32_t queueFamilyIndex,
		VkCommandPoolCreateFlags flags);

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
		VkCommandPool pool,
		uint32_t count);
} // namesapce vk_init_helpers
