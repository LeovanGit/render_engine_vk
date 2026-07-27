#include "render_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

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

    m_instance->InitVulkan();

    m_instance->InitSwapchain();

    m_instance->InitCommands();

    m_instance->InitSyncStructures();
}

void RenderEngine::InitVulkan()
{
    // vkInstance
    vkb::InstanceBuilder builder;

    auto instanceRet = builder
        .set_app_name("Example Vulkan Application")
        .request_validation_layers(bUseValidationLayers)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance vkbInstance = instanceRet.value();

    m_vkInstance = vkbInstance.instance;
    m_debugMessenger = vkbInstance.debug_messenger;

    // VkSurfaceKHR
    SDL_Vulkan_CreateSurface(m_window, m_vkInstance, &m_surface);

    // vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    // vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // select GPU
    vkb::PhysicalDeviceSelector selector{ vkbInstance };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features13)
        .set_required_features_12(features12)
        .set_surface(m_surface)
        .select()
        .value();

    // VkPhysicalDevice and VkDevice
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    m_device = vkbDevice.device;
    m_chosenGPU = physicalDevice.physical_device;
}

void RenderEngine::InitSwapchain()
{
    CreateSwapchain(m_swapchainSize.width, m_swapchainSize.height);
}

void RenderEngine::CreateSwapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder{ m_chosenGPU, m_device, m_surface };

    m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ m_swapchainImageFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    m_swapchainSize = vkbSwapchain.extent;
    m_swapchain = vkbSwapchain.swapchain;
    m_swapchainImages = vkbSwapchain.get_images().value();
    m_swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void RenderEngine::DestroySwapchain()
{
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    for (int i = 0; i != m_swapchainImageViews.size(); i++)
    {
        vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
    }
}

void RenderEngine::InitCommands()
{

}

void RenderEngine::InitSyncStructures()
{

}

void RenderEngine::Destroy()
{
    assert(m_instance != nullptr && "Engine is not initialized");

    m_instance->DestroySwapchain();

    vkDestroySurfaceKHR(m_instance->m_vkInstance, m_instance->m_surface, nullptr);
    vkDestroyDevice(m_instance->m_device, nullptr);

    vkb::destroy_debug_utils_messenger(m_instance->m_vkInstance, m_instance->m_debugMessenger);
    vkDestroyInstance(m_instance->m_vkInstance, nullptr);

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
