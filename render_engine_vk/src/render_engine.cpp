#include "render_engine.h"

#include <SDL.h>

#include <assert.h>

#include <VkBootstrap.h>

RenderEngine* RenderEngine::m_instance = nullptr;

RenderEngine& RenderEngine::Get()
{
    assert(m_instance != nullptr && "Engine is not initialized");
    return *m_instance;
}

void RenderEngine::Init()
{
    assert(m_instance == nullptr && "Engine is initialized already");
    m_instance = new RenderEngine;

    SDL_Init(SDL_INIT_VIDEO);

    m_instance->m_window = SDL_CreateWindow(
        "Vulkan Engine",
        windowPos.x,
        windowPos.y,
        windowSize.width,
        windowSize.height,
        SDL_WINDOW_VULKAN);
}

void RenderEngine::Destroy()
{
    assert(m_instance != nullptr && "Engine is not initialized");

    SDL_DestroyWindow(m_instance->m_window);

    delete m_instance;
    m_instance = nullptr;
}

void RenderEngine::Run()
{
    SDL_Event event;
    bool run = true;

    while (run)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                run = false;
                break;
            }
        }

        Draw();
    }
}

void RenderEngine::Draw()
{

}
