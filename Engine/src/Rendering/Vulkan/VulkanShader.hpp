#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanRenderpass.hpp"

#include <vulkan/vulkan.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace Sphynx::Rendering {
	struct ShaderReflectionInfo {
		struct DescriptorBinding {
			vk::DescriptorType Type = {};
			vk::ShaderStageFlags Stage = {};
			std::string Name;

			size_t UniformSize = 0;
		};

		std::map<uint32_t, DescriptorBinding> DescriptorBindings; // binding - type pair
	};

	class SE_API SpirvHelper {
	public:
		static vk::Format SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32_t elements);

		static void GetReflectionInfo(const std::vector<uint32_t>& spirvCode, vk::ShaderStageFlags stage, ShaderReflectionInfo& outInfo);
	};

	struct VertexInput {
		vk::VertexInputBindingDescription Description{};
		std::vector<vk::VertexInputAttributeDescription> Attributes;
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

		void Bind(vk::CommandBuffer commandBuffer);

	private:
		vk::Pipeline m_Pipeline;
		vk::PipelineLayout m_PipelineLayout;

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
		static vk::Format ToVkFormat(AttributeFormat format);

		void Add(AttributeFormat attributeFormat, uint32_t offset, uint32_t binding = 0);

		const std::vector<vk::VertexInputAttributeDescription>& GetAttributes() const { return m_Attributes; }

	private:
		uint32_t m_Location = 0;
		std::vector<vk::VertexInputAttributeDescription> m_Attributes;
	};
}