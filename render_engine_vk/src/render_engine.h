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

private:
	RenderEngine() = default;
	~RenderEngine() = default;

	RenderEngine(RenderEngine& other) = delete;
	RenderEngine(RenderEngine&& other) = delete;

	void operator=(RenderEngine& other) = delete;
	void operator=(RenderEngine&& other) = delete;

	static RenderEngine* m_instance;
};
