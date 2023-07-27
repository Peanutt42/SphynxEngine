#include "pch.hpp"
#include "VulkanShader.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
#pragma region SpirvHelper
    VkFormat SpirvHelper::SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32_t elements) {
        switch (type) {
        default: return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::SByte:
            if (elements == 1) return VK_FORMAT_R8_SINT;
            if (elements == 2) return VK_FORMAT_R8G8_SINT;
            if (elements == 3) return VK_FORMAT_R8G8B8_SINT;
            if (elements == 4) return VK_FORMAT_R8G8B8A8_SINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::UByte:
            if (elements == 1) return VK_FORMAT_R8_UINT;
            if (elements == 2) return VK_FORMAT_R8G8_UINT;
            if (elements == 3) return VK_FORMAT_R8G8B8_UINT;
            if (elements == 4) return VK_FORMAT_R8G8B8A8_UINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::Short:
            if (elements == 1) return VK_FORMAT_R16_SINT;
            if (elements == 2) return VK_FORMAT_R16G16_SINT;
            if (elements == 3) return VK_FORMAT_R16G16B16_SINT;
            if (elements == 4) return VK_FORMAT_R16G16B16A16_SINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::UShort:
            if (elements == 1) return VK_FORMAT_R16_UINT;
            if (elements == 2) return VK_FORMAT_R16G16_UINT;
            if (elements == 3) return VK_FORMAT_R16G16B16_UINT;
            if (elements == 4) return VK_FORMAT_R16G16B16A16_UINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::Int:
            if (elements == 1) return VK_FORMAT_R32_SINT;
            if (elements == 2) return VK_FORMAT_R32G32_SINT;
            if (elements == 3) return VK_FORMAT_R32G32B32_SINT;
            if (elements == 4) return VK_FORMAT_R32G32B32A32_SINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::UInt:
            if (elements == 1) return VK_FORMAT_R32_UINT;
            if (elements == 2) return VK_FORMAT_R32G32_UINT;
            if (elements == 3) return VK_FORMAT_R32G32B32_UINT;
            if (elements == 4) return VK_FORMAT_R32G32B32A32_UINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::Int64:
            if (elements == 1) return VK_FORMAT_R64_SINT;
            if (elements == 2) return VK_FORMAT_R64G64_SINT;
            if (elements == 3) return VK_FORMAT_R64G64B64_SINT;
            if (elements == 4) return VK_FORMAT_R64G64B64A64_SINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::UInt64:
            if (elements == 1) return VK_FORMAT_R64_UINT;
            if (elements == 2) return VK_FORMAT_R64G64_UINT;
            if (elements == 3) return VK_FORMAT_R64G64B64_UINT;
            if (elements == 4) return VK_FORMAT_R64G64B64A64_UINT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::Float:
            if (elements == 1) return VK_FORMAT_R32_SFLOAT;
            if (elements == 2) return VK_FORMAT_R32G32_SFLOAT;
            if (elements == 3) return VK_FORMAT_R32G32B32_SFLOAT;
            if (elements == 4) return VK_FORMAT_R32G32B32A32_SFLOAT;
            return VK_FORMAT_UNDEFINED;
        case spirv_cross::SPIRType::BaseType::Double:
            if (elements == 1) return VK_FORMAT_R64_SFLOAT;
            if (elements == 2) return VK_FORMAT_R64G64_SFLOAT;
            if (elements == 3) return VK_FORMAT_R64G64B64_SFLOAT;
            if (elements == 4) return VK_FORMAT_R64G64B64A64_SFLOAT;
            return VK_FORMAT_UNDEFINED;
        }
    }

    void SpirvHelper::GetReflectionInfo(const std::vector<uint32_t>& spirvCode, VkShaderStageFlags stage, ShaderReflectionInfo& outInfo) {
        spirv_cross::Compiler compiler(spirvCode);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        for (auto& uniformBuffer : resources.uniform_buffers) {
            uint32_t binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
            std::string name = compiler.get_name(uniformBuffer.id);

            const auto& type = compiler.get_type(uniformBuffer.base_type_id);
            size_t size = compiler.get_declared_struct_size(type);
            outInfo.DescriptorBindings[binding] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage, name, size };
        }
        for (auto& sampler : resources.sampled_images) {
            uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
            std::string name = compiler.get_name(sampler.id);
            outInfo.DescriptorBindings[binding] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage, name };
        }
    }
#pragma endregion

    static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
        SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create shaderModule");
        
        return shaderModule;
    }

    VulkanShader::VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass)
        : m_SharedUniformBuffers(createInfo.SharedUniformBuffers)
    {
        SpirvHelper::GetReflectionInfo(createInfo.VertexCode, VK_SHADER_STAGE_VERTEX_BIT, m_ReflectionInfo);
        SpirvHelper::GetReflectionInfo(createInfo.VertexCode, VK_SHADER_STAGE_FRAGMENT_BIT, m_ReflectionInfo);

        VkShaderModule vertexModule = CreateShaderModule(VulkanContext::LogicalDevice, createInfo.VertexCode);
        VkShaderModule fragmentModule = CreateShaderModule(VulkanContext::LogicalDevice, createInfo.FragmentCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertexModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &createInfo.VertexInput.Description;
        vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)createInfo.VertexInput.Attributes.size();
        vertexInputInfo.pVertexAttributeDescriptions = createInfo.VertexInput.Attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VkResult result = vkCreatePipelineLayout(VulkanContext::LogicalDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create pipelineLayout");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
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

        result = vkCreateGraphicsPipelines(VulkanContext::LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
        SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create graphics pipeline");

        vkDestroyShaderModule(VulkanContext::LogicalDevice, fragmentModule, nullptr);
        vkDestroyShaderModule(VulkanContext::LogicalDevice, vertexModule, nullptr);


        for (const auto& [binding, descriptor] : m_ReflectionInfo.DescriptorBindings) {
            m_DescriptorNameToBindingMap[descriptor.Name] = binding;

            // Create empty uniform buffer (if not shared by other shaders)
            if (descriptor.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                !m_SharedUniformBuffers.contains(descriptor.Name))
            {
                // TODO: UniformBuffers
            }
        }
    }

    VulkanShader::~VulkanShader() {
        vkDestroyPipeline(VulkanContext::LogicalDevice, m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
        vkDestroyPipelineLayout(VulkanContext::LogicalDevice, m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    void VulkanShader::Bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }


    VkFormat VulkanVertexAttributeBuilder::ToVkFormat(AttributeFormat format) {
        switch (format) {
        default:
            throw std::invalid_argument("invalid format arg");
        case AttributeFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
        case AttributeFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
        case AttributeFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        }
    }

    void VulkanVertexAttributeBuilder::Add(AttributeFormat attributeFormat, uint32_t offset, uint32_t binding) {
        VkVertexInputAttributeDescription& desc = m_Attributes.emplace_back();
        desc.binding = binding;
        desc.location = m_Location;
        desc.format = ToVkFormat(attributeFormat);
        desc.offset = offset;
        
        m_Location++;
    }
}