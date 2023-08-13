#include "pch.hpp"
#include "VulkanShader.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
#pragma region SpirvHelper
    vk::Format SpirvHelper::SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32_t elements) {
        switch (type) {
        default: return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::SByte:
            if (elements == 1) return vk::Format::eR8Sint;
            if (elements == 2) return vk::Format::eR8G8Sint;
            if (elements == 3) return vk::Format::eR8G8B8Sint;
            if (elements == 4) return vk::Format::eR8G8B8A8Sint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::UByte:
            if (elements == 1) return vk::Format::eR8Uint;
            if (elements == 2) return vk::Format::eR8G8Uint;
            if (elements == 3) return vk::Format::eR8G8B8Uint;
            if (elements == 4) return vk::Format::eR8G8B8A8Uint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::Short:
            if (elements == 1) return vk::Format::eR16Sint;
            if (elements == 2) return vk::Format::eR16G16Sint;
            if (elements == 3) return vk::Format::eR16G16B16Sint;
            if (elements == 4) return vk::Format::eR16G16B16A16Sint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::UShort:
            if (elements == 1) return vk::Format::eR16Uint;
            if (elements == 2) return vk::Format::eR16G16Uint;
            if (elements == 3) return vk::Format::eR16G16B16Uint;
            if (elements == 4) return vk::Format::eR16G16B16A16Uint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::Int:
            if (elements == 1) return vk::Format::eR32Sint;
            if (elements == 2) return vk::Format::eR32G32Sint;
            if (elements == 3) return vk::Format::eR32G32B32Sint;
            if (elements == 4) return vk::Format::eR32G32B32A32Sint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::UInt:
            if (elements == 1) return vk::Format::eR32Uint;
            if (elements == 2) return vk::Format::eR32G32Uint;
            if (elements == 3) return vk::Format::eR32G32B32Uint;
            if (elements == 4) return vk::Format::eR32G32B32A32Uint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::Int64:
            if (elements == 1) return vk::Format::eR64Sint;
            if (elements == 2) return vk::Format::eR64G64Sint;
            if (elements == 3) return vk::Format::eR64G64B64Sint;
            if (elements == 4) return vk::Format::eR64G64B64A64Sint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::UInt64:
            if (elements == 1) return vk::Format::eR64Uint;
            if (elements == 2) return vk::Format::eR64G64Uint;
            if (elements == 3) return vk::Format::eR64G64B64Uint;
            if (elements == 4) return vk::Format::eR64G64B64A64Uint;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::Float:
            if (elements == 1) return vk::Format::eR32Sfloat;
            if (elements == 2) return vk::Format::eR32G32Sfloat;
            if (elements == 3) return vk::Format::eR32G32B32Sfloat;
            if (elements == 4) return vk::Format::eR32G32B32A32Sfloat;
            return vk::Format::eUndefined;
        case spirv_cross::SPIRType::BaseType::Double:
            if (elements == 1) return vk::Format::eR64Sfloat;
            if (elements == 2) return vk::Format::eR64G64Sfloat;
            if (elements == 3) return vk::Format::eR64G64B64Sfloat;
            if (elements == 4) return vk::Format::eR64G64B64A64Sfloat;
            return vk::Format::eUndefined;
        }
    }

    void SpirvHelper::GetReflectionInfo(const std::vector<uint32_t>& spirvCode, vk::ShaderStageFlags stage, ShaderReflectionInfo& outInfo) {
        spirv_cross::Compiler compiler(spirvCode);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        for (auto& uniformBuffer : resources.uniform_buffers) {
            uint32_t binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
            std::string name = compiler.get_name(uniformBuffer.id);

            const auto& type = compiler.get_type(uniformBuffer.base_type_id);
            size_t size = compiler.get_declared_struct_size(type);
            outInfo.DescriptorBindings[binding] = { vk::DescriptorType::eUniformBuffer, stage, name, size };
        }
        for (auto& sampler : resources.sampled_images) {
            uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
            std::string name = compiler.get_name(sampler.id);
            outInfo.DescriptorBindings[binding] = { vk::DescriptorType::eCombinedImageSampler, stage, name };
        }
    }
#pragma endregion

    static vk::ShaderModule CreateShaderModule(vk::Device device, const std::vector<uint32_t>& code) {
        vk::ShaderModuleCreateInfo createInfo{};
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        vk::ShaderModule shaderModule;
        vk::Result result = device.createShaderModule(&createInfo, nullptr, &shaderModule);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create shaderModule");
        
        return shaderModule;
    }

    VulkanShader::VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass)
        : m_SharedUniformBuffers(createInfo.SharedUniformBuffers)
    {
        SpirvHelper::GetReflectionInfo(createInfo.VertexCode, vk::ShaderStageFlagBits::eVertex, m_ReflectionInfo);
        SpirvHelper::GetReflectionInfo(createInfo.VertexCode, vk::ShaderStageFlagBits::eFragment, m_ReflectionInfo);

        vk::ShaderModule vertexModule = CreateShaderModule(VulkanContext::LogicalDevice, createInfo.VertexCode);
        vk::ShaderModule fragmentModule = CreateShaderModule(VulkanContext::LogicalDevice, createInfo.FragmentCode);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = vertexModule;
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = fragmentModule;
        fragShaderStageInfo.pName = "main";

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &createInfo.VertexInput.Description;
        vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)createInfo.VertexInput.Attributes.size();
        vertexInputInfo.pVertexAttributeDescriptions = createInfo.VertexInput.Attributes.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;

        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
                                              vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB |
                                              vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        vk::Result result = VulkanContext::LogicalDevice.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create pipelineLayout");

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.stageCount = (uint32_t)shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = renderpass.GetHandle();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        result = VulkanContext::LogicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create graphics pipeline");

        VulkanContext::LogicalDevice.destroyShaderModule(fragmentModule, nullptr);
        VulkanContext::LogicalDevice.destroyShaderModule(vertexModule, nullptr);


        for (const auto& [binding, descriptor] : m_ReflectionInfo.DescriptorBindings) {
            m_DescriptorNameToBindingMap[descriptor.Name] = binding;

            // Create empty uniform buffer (if not shared by other shaders)
            if (descriptor.Type == vk::DescriptorType::eUniformBuffer &&
                !m_SharedUniformBuffers.contains(descriptor.Name))
            {
                // TODO: UniformBuffers
            }
        }
    }

    VulkanShader::~VulkanShader() {
        VulkanContext::LogicalDevice.destroyPipeline(m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
        VulkanContext::LogicalDevice.destroyPipelineLayout(m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    void VulkanShader::Bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
    }


    vk::Format VulkanVertexAttributeBuilder::ToVkFormat(AttributeFormat format) {
        switch (format) {
        default:
            throw std::invalid_argument("invalid format arg");
        case AttributeFormat::Float2: return vk::Format::eR32G32Sfloat;
        case AttributeFormat::Float3: return vk::Format::eR32G32B32Sfloat;
        case AttributeFormat::Float4: return vk::Format::eR32G32B32A32Sfloat;
        }
    }

    void VulkanVertexAttributeBuilder::Add(AttributeFormat attributeFormat, uint32_t offset, uint32_t binding) {
        vk::VertexInputAttributeDescription& desc = m_Attributes.emplace_back();
        desc.binding = binding;
        desc.location = m_Location;
        desc.format = ToVkFormat(attributeFormat);
        desc.offset = offset;
        
        m_Location++;
    }
}