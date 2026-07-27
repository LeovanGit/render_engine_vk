#pragma once

#include "types.h"

namespace
{
	constexpr VkExtent2D windowSize = { 480 , 270 };
	constexpr VkOffset2D windowPos = { 1000, 200 };

	constexpr bool bUseValidationLayers = true;
} // namespace

// Singletone
class RenderEngine
{
public:
	static void Init();

	static void Destroy();

	static RenderEngine& Get();

	void Run(); // main loop

	void Draw();

	struct SDL_Window* m_window = nullptr;

	VkInstance m_vkInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_chosenGPU;
	VkDevice m_device;
	

private:
	RenderEngine() = default;
	~RenderEngine() = default;

	RenderEngine(RenderEngine& other) = delete;
	RenderEngine(RenderEngine&& other) = delete;

	void operator=(RenderEngine& other) = delete;
	void operator=(RenderEngine&& other) = delete;

	void InitVulkan();

	void InitSwapchain();

	void InitCommands();

	void InitSyncStructures();

	static RenderEngine* m_instance;
};
