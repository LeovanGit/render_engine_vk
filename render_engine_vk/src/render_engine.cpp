#include "render_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <assert.h>

#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "vk_helpers.h"

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

    m_instance->m_mainDeletionQueue.PushFunction([&]() { SDL_DestroyWindow(m_instance->m_window); });

    m_instance->InitVulkan();

    m_instance->InitSwapchain();

    m_instance->InitCommands();

    m_instance->InitSyncStructures();

    m_instance->InitDescriptors();

    m_instance->InitPipelines();
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
    m_mainDeletionQueue.PushFunction([&]() { vkDestroyInstance(m_vkInstance, nullptr); });

    m_debugMessenger = vkbInstance.debug_messenger;
    m_mainDeletionQueue.PushFunction([&]() { vkb::destroy_debug_utils_messenger(m_vkInstance, m_debugMessenger); });
    
    // VkSurfaceKHR
    SDL_Vulkan_CreateSurface(m_window, m_vkInstance, &m_surface);
    m_mainDeletionQueue.PushFunction([&]() { vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr); });

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
    m_mainDeletionQueue.PushFunction([&]() { vkDestroyDevice(m_device, nullptr); });

    m_chosenGPU = physicalDevice.physical_device;

    // Request VkQueue and QueueFamily index for QueueType::graphics (which support all commands):
    m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_graphicsQueueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Create VMA memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_chosenGPU;
    allocatorInfo.device = m_device;
    allocatorInfo.instance = m_vkInstance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(&allocatorInfo, &m_allocator);

    m_mainDeletionQueue.PushFunction([&](){ vmaDestroyAllocator(m_allocator); });
}

void RenderEngine::InitSwapchain()
{
    CreateSwapchain(m_swapchainSize.width, m_swapchainSize.height);

    // Create Render Target texture (VkImage):
    VkExtent3D drawImageExtent =
    {
        windowSize.width,
        windowSize.height,
        1
    };

    m_renderTarget.m_imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_renderTarget.m_imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo imgCreateInfo = vk_helpers::ImageCreateInfo(m_renderTarget.m_imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo imgAllocInfo = {};
    imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_allocator, &imgCreateInfo, &imgAllocInfo, &m_renderTarget.m_image, &m_renderTarget.m_allocation, nullptr);

    VkImageViewCreateInfo imageViewInfo = vk_helpers::ImageViewCreateInfo(m_renderTarget.m_imageFormat, m_renderTarget.m_image, VK_IMAGE_ASPECT_COLOR_BIT);
    VK_CHECK(vkCreateImageView(m_device, &imageViewInfo, nullptr, &m_renderTarget.m_imageView));

    m_mainDeletionQueue.PushFunction([=]() {
        vkDestroyImageView(m_device, m_renderTarget.m_imageView, nullptr);
        vmaDestroyImage(m_allocator, m_renderTarget.m_image, m_renderTarget.m_allocation);
     });
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

    m_mainDeletionQueue.PushFunction([&]() { DestroySwapchain(); });
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
    VkCommandPoolCreateInfo m_commandPoolInfo = vk_helpers::CommandPoolCreateInfo(m_graphicsQueueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i != FRAMEDATA_COUNT; i++)
    {
        VK_CHECK(vkCreateCommandPool(m_device, &m_commandPoolInfo, nullptr, &m_frames[i].m_commandPool));
        m_mainDeletionQueue.PushFunction([&, commandPool=m_frames[i].m_commandPool]() { vkDestroyCommandPool(m_device, commandPool, nullptr); });

        VkCommandBufferAllocateInfo cmdAllocInfo = vk_helpers::CommandBufferAllocateInfo(m_frames[i].m_commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i].m_commandBuffer));
    }

}

void RenderEngine::InitSyncStructures()
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;
    
    for (int i = 0; i != FRAMEDATA_COUNT; ++i)
    {
        VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_frames[i].m_renderFence));
        m_mainDeletionQueue.PushFunction([&, renderFence=m_frames[i].m_renderFence]() { vkDestroyFence(m_device, renderFence, nullptr); });
    
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i].m_swapchainSemaphore));
        m_mainDeletionQueue.PushFunction([&, swapchainSemaphore=m_frames[i].m_swapchainSemaphore]() { vkDestroySemaphore(m_device, swapchainSemaphore, nullptr); });
    
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i].m_renderSemaphore));
        m_mainDeletionQueue.PushFunction([&, renderSemaphore=m_frames[i].m_renderSemaphore]() { vkDestroySemaphore(m_device, renderSemaphore, nullptr); });
    }
}

void RenderEngine::InitDescriptors()
{
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };

    m_globalDescriptorAllocator.InitPool(m_device, 10, sizes);

    {
        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_drawImageDescriptorLayout = builder.Build(m_device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    m_drawImageDescriptors = m_globalDescriptorAllocator.Allocate(m_device, m_drawImageDescriptorLayout);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = m_renderTarget.m_imageView;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;
    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = m_drawImageDescriptors;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(m_device, 1, &drawImageWrite, 0, nullptr);

    m_mainDeletionQueue.PushFunction([&]()
        {
            m_globalDescriptorAllocator.DestroyPool(m_device);
            vkDestroyDescriptorSetLayout(m_device, m_drawImageDescriptorLayout, nullptr);
        });
}

void RenderEngine::InitPipelines()
{
    InitBackgroundPipelines();
}

void RenderEngine::InitBackgroundPipelines()
{
    VkPipelineLayoutCreateInfo computeLayout = {};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &m_drawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    VK_CHECK(vkCreatePipelineLayout(m_device, &computeLayout, nullptr, &m_gradientPipelineLayout));

    VkShaderModule computeDrawShader;
    if (!vk_helpers::LoadShaderModule("x64/Debug/shaders/gradient.spv", m_device, &computeDrawShader))
    {
        fmt::print("Error when building the compute shader \n");
        assert(false && "Error when building the compute shader \n");
    }

    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.pNext = nullptr;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = computeDrawShader;
    stageInfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_gradientPipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_gradientPipeline));

    vkDestroyShaderModule(m_device, computeDrawShader, nullptr);
    
    m_mainDeletionQueue.PushFunction(
        [&]()
        {
            vkDestroyPipelineLayout(m_device, m_gradientPipelineLayout, nullptr);
            vkDestroyPipeline(m_device, m_gradientPipeline, nullptr);
        });
}

void RenderEngine::Destroy()
{
    assert(m_instance != nullptr && "Engine is not initialized");

    vkDeviceWaitIdle(m_instance->m_device);

    m_instance->m_mainDeletionQueue.Flush();

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

void RenderEngine::DrawBackground(VkCommandBuffer cmd)
{
    //VkClearColorValue clearValue;
    //float flash = std::abs(std::sin(m_currentFrameNumber / 120.f));
    //clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
    //
    //VkImageSubresourceRange clearRange = vk_helpers::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    //
    //vkCmdClearColorImage(cmd, m_renderTarget.m_image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // Биндим наш VkPipeline:
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gradientPipeline);

    // Биндим VkPipelineLayout и VkDesctiptorSet:
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_gradientPipelineLayout, 0, 1, &m_drawImageDescriptors, 0, nullptr);

    // Дроуколл:
    // Поскольку мы в шейдере указали размер рабочей группы 16х16 пикселей, то
    // нам нужно вызвать (width / 16, height / 16) рабочих групп,
    // только с округлением вверх (на случай если размер текстуры не
    // делится нацело на размер рабочей группы):
    vkCmdDispatch(
        cmd,
        std::ceil(m_renderTarget.m_imageExtent.width / 16.0f),
        std::ceil(m_renderTarget.m_imageExtent.height / 16.0f),
        1);
}

void RenderEngine::Draw()
{
    VK_CHECK(vkWaitForFences(m_device, 1, &GetCurrentFrame().m_renderFence, true, 1000000000));

    GetCurrentFrame().m_deletionQueue.Flush();

    VK_CHECK(vkResetFences(m_device, 1, &GetCurrentFrame().m_renderFence));

    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, GetCurrentFrame().m_swapchainSemaphore, nullptr, &swapchainImageIndex));

    VkCommandBuffer cmd = GetCurrentFrame().m_commandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    vk_helpers::TransitionImage(cmd, m_renderTarget.m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    DrawBackground(cmd);

    vk_helpers::TransitionImage(cmd, m_renderTarget.m_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vk_helpers::TransitionImage(cmd, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkExtent2D renderTargetSize = {};
    renderTargetSize.width = m_renderTarget.m_imageExtent.width;
    renderTargetSize.height = m_renderTarget.m_imageExtent.height;
    vk_helpers::CopyImageToImage(cmd, m_renderTarget.m_image, m_swapchainImages[swapchainImageIndex], renderTargetSize, m_swapchainSize);

    vk_helpers::TransitionImage(cmd, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdinfo = vk_helpers::CommandBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = vk_helpers::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().m_swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = vk_helpers::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().m_renderSemaphore);

    VkSubmitInfo2 submitInfo = vk_helpers::SubmitInfo(&cmdinfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, GetCurrentFrame().m_renderFence));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &GetCurrentFrame().m_renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(m_graphicsQueue, &presentInfo));

    ++m_currentFrameNumber;
}
