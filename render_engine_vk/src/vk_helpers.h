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

	bool LoadShaderModule(
		const char* filePath,
		VkDevice device,
		VkShaderModule* outShaderModule);

	VkRenderingAttachmentInfo AttachmentInfo(
		VkImageView view,
		VkClearValue* clear,
		VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/);

	VkRenderingInfo RenderingInfo(
		VkExtent2D colorAttachmentExtent,
		VkRenderingAttachmentInfo* colorAttachmentInfo,
		VkRenderingAttachmentInfo* depthAttachmentInfo);
} // namesapce vk_init_helpers
