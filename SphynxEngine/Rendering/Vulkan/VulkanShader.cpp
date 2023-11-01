#include "pch.hpp"
#include "VulkanShader.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
#pragma region SpirvHelper
    vk::Format SpirvHelper::SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32 elements) {
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

    void SpirvHelper::GetReflectionInfo(const std::vector<uint32>& spirvCode, vk::ShaderStageFlags stage, ShaderReflectionInfo& outInfo) {
        spirv_cross::Compiler compiler(spirvCode.data(), spirvCode.size());
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        for (auto& uniformBuffer : resources.uniform_buffers) {
            uint32 binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
            const std::string& name = compiler.get_name(uniformBuffer.id);

            const auto& type = compiler.get_type(uniformBuffer.base_type_id);
            size_t size = compiler.get_declared_struct_size(type);
            outInfo.DescriptorBindings[binding] = { vk::DescriptorType::eUniformBuffer, stage, name, size };
        }
        for (auto& sampler : resources.sampled_images) {
            uint32 binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
            const std::string& name = compiler.get_name(sampler.id);
            outInfo.DescriptorBindings[binding] = { vk::DescriptorType::eCombinedImageSampler, stage, name };
        }

        if (stage & vk::ShaderStageFlagBits::eVertex) {
            outInfo.VertexAttributes.clear();
            for (auto& input : resources.stage_inputs) {
                const auto& type = compiler.get_type(input.type_id);
                uint32_t location = compiler.get_decoration(input.id, spv::DecorationLocation);

                vk::Format attributeFormat = SpirvTypeToVkFormat(type.basetype, type.vecsize);
                for (uint32_t i = 0; i < type.columns; i++) {
                    uint32_t attributeSize = (type.width * type.vecsize) / 8;
                    uint32_t attributeLocation = location + i;

                    vk::VertexInputAttributeDescription& desc = outInfo.VertexAttributes.emplace_back();
                    desc.binding = 0;
                    desc.location = attributeLocation;
                    desc.format = attributeFormat;
                    outInfo.VertexInputAttributeSizes[attributeLocation] = attributeSize;
                }
            }
        }
    }
#pragma endregion

    static vk::ShaderModule CreateShaderModule(vk::Device device, const std::vector<uint32>& code) {
        vk::ShaderModuleCreateInfo createInfo{};
        createInfo.codeSize = code.size() * sizeof(uint32);
        createInfo.pCode = (const uint32*)code.data();
        vk::ShaderModule shaderModule;
        vk::Result result = device.createShaderModule(&createInfo, nullptr, &shaderModule);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create shaderModule");
        
        return shaderModule;
    }

    VulkanShader::VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass)
        : m_ReflectionInfo(createInfo.ReflectionInfo)
    {
        vk::ShaderModule vertexModule = CreateShaderModule(VulkanContext::LogicalDevice, createInfo.VertexCode);
        vk::ShaderModule fragmentModule = CreateShaderModule(VulkanContext::LogicalDevice, createInfo.FragmentCode);

        // Descritpor set layout
        std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(m_ReflectionInfo.DescriptorBindings.size());
        for (const auto& [binding, descriptor] : m_ReflectionInfo.DescriptorBindings) {
            auto& layoutBinding = layoutBindings.emplace_back();
            layoutBinding.binding = binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = descriptor.Type;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = descriptor.Stage;
        }
        vk::DescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.bindingCount = (uint32)layoutBindings.size();
        layoutInfo.pBindings = layoutBindings.data();

        vk::Result result = VulkanContext::LogicalDevice.createDescriptorSetLayout(&layoutInfo, nullptr, &m_DescriptorSetLayout);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create descriptor set layout");

        // Descriptor sets
        std::vector<vk::DescriptorSetLayout> layouts(VulkanContext::MaxFramesInFlight, m_DescriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.descriptorPool = VulkanContext::DescriptorPool;
        allocInfo.descriptorSetCount = VulkanContext::MaxFramesInFlight;
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(VulkanContext::MaxFramesInFlight);
        result = VulkanContext::LogicalDevice.allocateDescriptorSets(&allocInfo, m_DescriptorSets.data());
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to allocate descriptor sets");


        // Pipeline Layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        result = VulkanContext::LogicalDevice.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create pipelineLayout");

        // Pipeline
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = (uint32)createInfo.VertexInputBindings.size();
        vertexInputInfo.pVertexBindingDescriptions = createInfo.VertexInputBindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = (uint32)m_ReflectionInfo.VertexAttributes.size();
        vertexInputInfo.pVertexAttributeDescriptions = m_ReflectionInfo.VertexAttributes.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = createInfo.Wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
                                              vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB |
                                              vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.dynamicStateCount = (uint32)dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();


        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = vertexModule;
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = fragmentModule;
        fragShaderStageInfo.pName = "main";

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
        
        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.stageCount = (uint32)shaderStages.size();
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

        result = VulkanContext::LogicalDevice.createGraphicsPipelines(VulkanContext::PipelineCache->GetHandle(), 1, &pipelineInfo, nullptr, &m_Pipeline);
        SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create graphics pipeline");

        VulkanContext::LogicalDevice.destroyShaderModule(fragmentModule, nullptr);
        VulkanContext::LogicalDevice.destroyShaderModule(vertexModule, nullptr);


        for (const auto& [binding, descriptor] : m_ReflectionInfo.DescriptorBindings) {
            m_DescriptorNameToBindingMap[descriptor.Name] = binding;

            // Create empty uniform buffer (if not shared by other shaders)
            if (descriptor.Type == vk::DescriptorType::eUniformBuffer &&
                !ShaderCreateInfo::SharedUniformBuffers.contains(descriptor.Name))
            {
                m_UniformBuffers[binding] = std::make_unique<VulkanUniformBuffer>(descriptor.UniformSize);
                SetUniformBuffer(descriptor.Name, *m_UniformBuffers.at(binding));
            }
        }
    }

    VulkanShader::~VulkanShader() {
        m_UniformBuffers.clear();

        VulkanContext::LogicalDevice.destroyDescriptorSetLayout(m_DescriptorSetLayout, nullptr);

        VulkanContext::LogicalDevice.destroyPipeline(m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
        VulkanContext::LogicalDevice.destroyPipelineLayout(m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    void VulkanShader::SetUniformBuffer(const std::string& name, const VulkanUniformBuffer& uniformBuffer) {
        std::optional<uint32> binding = _GetBinding(name);
        if (!binding) {
            SE_ERR(Logging::Rendering, "Failed to set uniform buffer, '{}' doesn't exist", name);
            return;
        }
    
        std::vector<vk::Buffer> buffers = uniformBuffer.GetBuffers();
        for (size_t i = 0; i < m_DescriptorSets.size(); i++) {

            vk::WriteDescriptorSet descriptorWrite{};
            descriptorWrite.dstSet = m_DescriptorSets[i];
            descriptorWrite.dstBinding = *binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            vk::DescriptorBufferInfo bufferInfo{ buffers[i], 0, (uint32)uniformBuffer.Size };
            descriptorWrite.pBufferInfo = &bufferInfo;

            VulkanContext::LogicalDevice.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
        }
    }

    void VulkanShader::SetImageSampler(const std::string& name, vk::Sampler sampler, const std::vector<vk::ImageView>& imageViews) {
        std::optional<uint32> binding = _GetBinding(name);
        if (!binding) {
            SE_ERR(Logging::Rendering, "Failed to set image sampler, '{}' doesn't exist", name);
            return;
        }

        for (size_t i = 0; i < m_DescriptorSets.size(); i++) {
            vk::WriteDescriptorSet descriptorWrite{};
            descriptorWrite.dstSet = m_DescriptorSets[i];
            descriptorWrite.dstBinding = *binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;

            vk::DescriptorImageInfo imageInfo{ sampler, imageViews[i], vk::ImageLayout::eShaderReadOnlyOptimal };
            descriptorWrite.pImageInfo = &imageInfo;
            VulkanContext::LogicalDevice.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
        }
    }

    void VulkanShader::Bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, 1, &m_DescriptorSets[VulkanContext::CurrentFrame], 0, nullptr);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
    }

    std::optional<uint32> VulkanShader::_GetBinding(const std::string& name) {
        auto find = m_DescriptorNameToBindingMap.find(name);
        if (find == m_DescriptorNameToBindingMap.end())
            return std::nullopt;
        return find->second;
    }
}