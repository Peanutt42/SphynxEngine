#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanRenderpass.hpp"

#include <vulkan/vulkan.h>
#include <spirv_cross/spirv_cross.hpp>

namespace Sphynx::Rendering {
	struct ShaderReflectionInfo {
		struct DescriptorBinding {
			VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
			VkShaderStageFlags Stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			std::string Name;

			size_t UniformSize = 0;
		};

		std::map<uint32_t, DescriptorBinding> DescriptorBindings; // binding - type pair
	};

	class SE_API SpirvHelper {
	public:
		static VkFormat SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32_t elements);

		static void GetReflectionInfo(const std::vector<uint32_t>& spirvCode, VkShaderStageFlags stage, ShaderReflectionInfo& outInfo);
	};

	struct VertexInput {
		VkVertexInputBindingDescription Description{};
		std::vector<VkVertexInputAttributeDescription> Attributes;
	};

	struct SE_API ShaderCreateInfo {
		std::vector<uint32_t> VertexCode;
		std::vector<uint32_t> FragmentCode;

		// Names of uniform buffers that are shared with other shaders and so aren't auto created
		std::unordered_set<std::string> SharedUniformBuffers = {
			//"u_UBO",
		};

		VertexInput VertexInput;

		bool Wireframe = false;
		bool DepthTesting = true;
	};

	class VulkanShader {
	public:
		VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass);
		~VulkanShader();

		void Bind(VkCommandBuffer commandBuffer);

	private:
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		ShaderReflectionInfo m_ReflectionInfo;
		std::unordered_map<std::string, uint32_t> m_DescriptorNameToBindingMap;

		std::unordered_set<std::string> m_SharedUniformBuffers;
	};

	enum class AttributeFormat {
		Float2,
		Float3,
		Float4
	};
	class VulkanVertexAttributeBuilder {
	public:
		static VkFormat ToVkFormat(AttributeFormat format);

		void Add(AttributeFormat attributeFormat, uint32_t offset, uint32_t binding = 0);

		const std::vector<VkVertexInputAttributeDescription>& GetAttributes() const { return m_Attributes; }

	private:
		uint32_t m_Location = 0;
		std::vector<VkVertexInputAttributeDescription> m_Attributes;
	};
}