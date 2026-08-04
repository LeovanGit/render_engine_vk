#pragma once

#include "types.h"

namespace
{
	constexpr VkExtent2D windowSize = { 480 , 270 };
	constexpr VkOffset2D windowPos = { 1000, 200 };

	constexpr bool bUseValidationLayers = true;

	constexpr unsigned int FRAMEDATA_COUNT = 2;
} // namespace

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void PushFunction(std::function<void()>&& function)
	{
		deletors.push_back(function);
	}

	void Flush()
	{
		for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
		{
			(*it)();
		}

		deletors.clear();
	}
};

struct FrameData
{
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;

	VkFence m_renderFence;
	VkSemaphore m_swapchainSemaphore;
	VkSemaphore m_renderSemaphore;

	DeletionQueue m_deletionQueue;
};

// Singletone
class RenderEngine
{
public:
	static void Init();

	static void Destroy();

	static RenderEngine& Get();

	void Run(); // main loop

	void Draw();

	FrameData& GetCurrentFrame()
	{
		return m_frames[m_currentFrameNumber % FRAMEDATA_COUNT];
	}

	DeletionQueue m_mainDeletionQueue;

	struct SDL_Window* m_window = nullptr;

	VkInstance m_vkInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_chosenGPU;
	VkDevice m_device;

	VkSwapchainKHR m_swapchain;
	VkFormat m_swapchainImageFormat;

	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkExtent2D m_swapchainSize;

	uint32_t m_currentFrameNumber = 0;

	FrameData m_frames[FRAMEDATA_COUNT];

	VkQueue m_graphicsQueue;
	uint32_t m_graphicsQueueFamilyIndex;
	
private:
	RenderEngine() = default;
	~RenderEngine() = default;

	RenderEngine(RenderEngine& other) = delete;
	RenderEngine(RenderEngine&& other) = delete;

	void operator=(RenderEngine& other) = delete;
	void operator=(RenderEngine&& other) = delete;

	void InitVulkan();

	void InitSwapchain();
	void CreateSwapchain(uint32_t width, uint32_t height);
	void DestroySwapchain();

	void InitCommands();

	void InitSyncStructures();

	static RenderEngine* m_instance;
};
