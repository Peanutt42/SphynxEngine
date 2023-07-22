#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanRenderpass.hpp"

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace Sphynx::Rendering {
	class SpirvHelper {
	public:
		static void CompileShader(const std::string& shaderCode, shaderc_shader_kind type, const std::string& shaderName, std::vector<uint32_t>& outSpirvCode);

		struct ShaderReflectionInfo {
			std::vector<VkVertexInputAttributeDescription> VertexAttributes;
			std::map<uint32_t, uint32_t> VertexInputAttributeSizes;

			struct DescriptorBinding {
				VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
				VkShaderStageFlags Stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
				std::string Name;

				// For Uniforms
				size_t Size = 0;
			};

			std::map<uint32_t, DescriptorBinding> DescriptorBindings; // binding - type pair
		};

		static VkFormat SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32_t elements);

		static void GetReflectionInfo(const std::vector<uint32_t>& spirvCode, VkShaderStageFlags stage, ShaderReflectionInfo& outInfo);
	};

	struct ShaderCreateInfo {
		ShaderCreateInfo() = default;
		ShaderCreateInfo(const std::vector<uint32_t>& vertexCode, const std::vector<uint32_t>& fragmentCode)
			: VertexCode(vertexCode), FragmentCode(fragmentCode) {}
		ShaderCreateInfo(const std::filesystem::path& filepath);

		std::vector<uint32_t> VertexCode;
		std::vector<uint32_t> FragmentCode;

		bool Wireframe = false;
		bool DepthTesting = true;
	};

	class VulkanShader {
	public:
		VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass);
		~VulkanShader();

		void Bind(VkCommandBuffer commandBuffer);

	private:
		ShaderCreateInfo m_CreateInfo;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	};
}