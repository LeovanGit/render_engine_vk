#pragma once

#include "types.h"

struct AllocatedImage
{
	VkImage m_image;
	VkImageView m_imageView;
	VmaAllocation m_allocation;
	VkExtent3D m_imageExtent;
	VkFormat m_imageFormat;
};

namespace vk_helpers
{
	VkImageCreateInfo ImageCreateInfo(
		VkFormat format,
		VkImageUsageFlags usageFlags,
		VkExtent3D extent);

	VkImageViewCreateInfo ImageViewCreateInfo(
		VkFormat format,
		VkImage image,
		VkImageAspectFlags aspectFlags);

	void CopyImageToImage(
		VkCommandBuffer cmd,
		VkImage source,
		VkImage destination,
		VkExtent2D srcSize,
		VkExtent2D dstSize);
} // namespace vk_helpers
