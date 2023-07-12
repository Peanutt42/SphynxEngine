#include "pch.hpp"
#include "VulkanShader.hpp"

namespace Sphynx::Rendering {
#pragma region SpirvHelper
	void SpirvHelper::CompileShader(const std::string& shaderCode, shaderc_shader_kind type, const std::string& shaderName, std::vector<uint32_t>& outSpirvCode) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
        options.SetGenerateDebugInfo();

        // Preprocessing
        shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(shaderCode, type, shaderName.c_str(), options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            SE_FATAL(Logging::Rendering, "Precompilation error: {}", result.GetErrorMessage());
            return;
        }
        std::string preprocessedShaderCode = { result.cbegin(), result.cend() };

        // Compiling
        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(preprocessedShaderCode, type, shaderName.c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            SE_FATAL(Logging::Rendering, "Failed to compile shader: {}", module.GetErrorMessage());
            return;
        }
        outSpirvCode.assign(module.cbegin(), module.cend());
        if (outSpirvCode.empty())
            SE_FATAL(Logging::Rendering, "Failed to compile shader");
    }

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

        if (stage & VK_SHADER_STAGE_VERTEX_BIT) {
            outInfo.VertexAttributes.clear();
            for (auto& input : resources.stage_inputs) {
                const auto& type = compiler.get_type(input.type_id);
                uint32_t location = compiler.get_decoration(input.id, spv::DecorationLocation);

                VkFormat attributeFormat = SpirvTypeToVkFormat(type.basetype, type.vecsize);
                for (uint32_t i = 0; i < type.columns; i++) {
                    uint32_t attributeSize = (type.width * type.vecsize) / 8;
                    uint32_t attributeLocation = location + i;

                    VkVertexInputAttributeDescription& desc = outInfo.VertexAttributes.emplace_back();
                    desc.binding = 0;
                    desc.location = attributeLocation;
                    desc.format = attributeFormat;
                    outInfo.VertexInputAttributeSizes[attributeLocation] = attributeSize;
                }
            }
        }
    }
#pragma endregion

#pragma region ShaderCreateInfo
    static void SeperateVertexAndFragmentShaderCode(const std::string& fileContents, std::string& vertexCode, std::string& fragmentCode) {
        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = fileContents.find(typeToken, 0); //Start of shader type declaration line
        while (pos != std::string::npos) {
            size_t eol = fileContents.find_first_of('\n', pos); //End of shader type declaration line
            SE_ASSERT(eol != std::string::npos, Logging::Rendering, "Syntax error");
            size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
            std::string type = fileContents.substr(begin, eol - begin);

            size_t nextLinePos = fileContents.find_first_not_of('\n', eol); //Start of shader code after shader type declaration line
            SE_ASSERT(nextLinePos != std::string::npos, Logging::Rendering, "Syntax error");
            pos = fileContents.find(typeToken, nextLinePos); //Start of next shader type declaration line

            if (type == "vertex") vertexCode = (pos == std::string::npos) ? fileContents.substr(nextLinePos) : fileContents.substr(nextLinePos, pos - nextLinePos);
            else if (type == "fragment") fragmentCode = (pos == std::string::npos) ? fileContents.substr(nextLinePos) : fileContents.substr(nextLinePos, pos - nextLinePos);
            else SE_ERR(Logging::Rendering, "Unkown shader type {}", type);
        }
    }

    static bool IsCachedShaderUsable(const std::filesystem::path& shaderFilepath, const std::filesystem::path& cachedShaderFilepath) {
        if (!std::filesystem::exists(cachedShaderFilepath))
            return false;

        if (std::filesystem::last_write_time(shaderFilepath) > std::filesystem::last_write_time(cachedShaderFilepath))
            return false;

        return true;
    }

    ShaderCreateInfo::ShaderCreateInfo(const std::filesystem::path& filepath) {
        std::filesystem::path cachedShaderFilepath = filepath;
        cachedShaderFilepath.replace_extension(".cached_shader");

        if (IsCachedShaderUsable(filepath, cachedShaderFilepath)) {
            std::ifstream shaderFile(cachedShaderFilepath, std::ios::binary);
            if (shaderFile) {
                size_t vertexShaderSize = 0;
                shaderFile.read((char*)&vertexShaderSize, sizeof(vertexShaderSize));
                VertexCode.resize(vertexShaderSize);
                shaderFile.read((char*)VertexCode.data(), vertexShaderSize * sizeof(uint32_t));

                size_t fragmentShaderSize = 0;
                shaderFile.read((char*)&fragmentShaderSize, sizeof(fragmentShaderSize));
                FragmentCode.resize(fragmentShaderSize);
                shaderFile.read((char*)FragmentCode.data(), fragmentShaderSize * sizeof(uint32_t));
            }
            else
                SE_ERR(Logging::Rendering, "Failed to load cached shader file {}", cachedShaderFilepath.string());
        }
        else {
            ScopedTimer t("Compiling shader " + filepath.string());

            std::ifstream shaderFile(filepath);
            std::stringstream ss;
            ss << shaderFile.rdbuf();
            std::string shaderCode = ss.str();
            std::string shaderName = filepath.filename().string();

            std::string vertexCode, fragmentCode;
            SeperateVertexAndFragmentShaderCode(shaderCode, vertexCode, fragmentCode);

            SpirvHelper::CompileShader(vertexCode, shaderc_vertex_shader, shaderName, VertexCode);
            SpirvHelper::CompileShader(fragmentCode, shaderc_fragment_shader, shaderName, FragmentCode);

            std::ofstream cachedShaderFile(cachedShaderFilepath, std::ios::binary);

            size_t vertexShaderSize = VertexCode.size();
            cachedShaderFile.write((char*)&vertexShaderSize, sizeof(size_t));
            cachedShaderFile.write((char*)VertexCode.data(), VertexCode.size() * sizeof(uint32_t));

            size_t fragmentShaderSize = FragmentCode.size();
            cachedShaderFile.write((char*)&fragmentShaderSize, sizeof(size_t));
            cachedShaderFile.write((char*)FragmentCode.data(), FragmentCode.size() * sizeof(uint32_t));
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

    VulkanShader::VulkanShader(const ShaderCreateInfo& createInfo, VkDevice device, VulkanRenderpass& renderpass)
        : m_CreateInfo(createInfo), m_Device(device)
    {
        VkShaderModule vertexModule = CreateShaderModule(m_Device, m_CreateInfo.VertexCode);
        VkShaderModule fragmentModule = CreateShaderModule(m_Device, m_CreateInfo.FragmentCode);

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
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

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

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
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

        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
        SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create graphics pipeline");

        vkDestroyShaderModule(device, fragmentModule, nullptr);
        vkDestroyShaderModule(device, vertexModule, nullptr);
    }

    VulkanShader::~VulkanShader() {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    void VulkanShader::Bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }
}