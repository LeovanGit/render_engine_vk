#pragma once

#include "types.h"

namespace vk_helpers
{
	VkCommandPoolCreateInfo CommandPoolCreateInfo(
		uint32_t queueFamilyIndex,
		VkCommandPoolCreateFlags flags);

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
		VkCommandPool pool,
		uint32_t count);

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask);

	void TransitionImage(
		VkCommandBuffer cmd,
		VkImage image,
		VkImageLayout currentLayout,
		VkImageLayout newLayout);

	VkSemaphoreSubmitInfo SemaphoreSubmitInfo(
		VkPipelineStageFlags2 stageMask,
		VkSemaphore semaphore);

	VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);

	// pass CommandBufferSubmitInfo() + SemaphoreSubmitInfo() here:
	VkSubmitInfo2 SubmitInfo(
		VkCommandBufferSubmitInfo* cmd,
		VkSemaphoreSubmitInfo* signalSemaphoreInfo,
		VkSemaphoreSubmitInfo* waitSemaphoreInfo);
} // namesapce vk_init_helpers
