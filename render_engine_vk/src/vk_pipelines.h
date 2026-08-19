#pragma once

#include "types.h"
#include "vk_helpers.h"

class PipelineBuilder
{
public:
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

    VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
    VkPipelineRasterizationStateCreateInfo m_rasterizer;
    VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo m_multisampling;
    VkPipelineDepthStencilStateCreateInfo m_depthStencil;

    VkPipelineRenderingCreateInfo m_renderInfo;
    VkFormat m_colorAttachmentFormat;

    VkPipelineLayout m_pipelineLayout;

    PipelineBuilder();

    void Clear();

    void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);

    void SetInputTopology(VkPrimitiveTopology topology);

    void SetPolygonMode(VkPolygonMode mode);

    void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);

    void DisableMultisampling();

    void DisableBlending();

    void SetColorAttachmentFormat(VkFormat format);

    void SetDepthFormat(VkFormat format);

    void DisableDepthTest();

    VkPipeline BuildPipeline(VkDevice device);
};
