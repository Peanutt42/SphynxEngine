#pragma once

#include "pch.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanUniformBuffer.hpp"

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

		std::map<uint32, DescriptorBinding> DescriptorBindings; // binding - type pair
	};

	class SE_API SpirvHelper {
	public:
		static vk::Format SpirvTypeToVkFormat(spirv_cross::SPIRType::BaseType type, uint32 elements);

		static void GetReflectionInfo(const std::vector<uint32>& spirvCode, vk::ShaderStageFlags stage, ShaderReflectionInfo& outInfo);
	};

	struct VertexInput {
		std::vector<vk::VertexInputBindingDescription> Bindings;
		std::vector<vk::VertexInputAttributeDescription> Attributes;
	};

	struct SE_API ShaderCreateInfo {
		std::vector<uint32> VertexCode;
		std::vector<uint32> FragmentCode;

		// Names of uniform buffers that are shared with other shaders and so aren't auto created
		const inline static std::unordered_set<std::string> SharedUniformBuffers = {
			"v_ubo",
			"f_ubo"
		};

		VertexInput VertexInput;

		bool Wireframe = false;
		bool DepthTesting = true;
	};

	class VulkanShader {
	public:
		VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass);
		~VulkanShader();

		void SetUniformBuffer(const std::string& name, const VulkanUniformBuffer& uniformBuffer);

		template<typename T>
		void UpdateUniformBuffer(const std::string& name, const T& data) {
			std::optional<uint32> binding = _GetBinding(name);
			if (binding)
				m_UniformBuffers.at(*binding)->Update<T>(data);
		}

		void Bind(vk::CommandBuffer commandBuffer);

	private:
		std::optional<uint32> _GetBinding(const std::string& name);

	private:
		vk::Pipeline m_Pipeline;
		vk::PipelineLayout m_PipelineLayout;

		ShaderReflectionInfo m_ReflectionInfo;
		std::unordered_map<std::string, uint32> m_DescriptorNameToBindingMap;

		vk::DescriptorSetLayout m_DescriptorSetLayout;
		std::vector<vk::DescriptorSet> m_DescriptorSets;

		std::unordered_map<uint32, std::unique_ptr<VulkanUniformBuffer>> m_UniformBuffers;
	};

	enum class AttributeFormat {
		Float2,
		Float3,
		Float4
	};
	class VulkanVertexAttributeBuilder {
	public:
		static vk::Format ToVkFormat(AttributeFormat format);

		void Add(AttributeFormat attributeFormat, uint32 offset, uint32 binding = 0);

		const std::vector<vk::VertexInputAttributeDescription>& GetAttributes() const { return m_Attributes; }

	private:
		uint32 m_Location = 0;
		std::vector<vk::VertexInputAttributeDescription> m_Attributes;
	};
}