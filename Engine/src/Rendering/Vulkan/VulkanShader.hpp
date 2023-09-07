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
		std::vector<vk::VertexInputAttributeDescription> VertexAttributes;
		std::map<uint32_t, uint32_t> VertexInputAttributeSizes;

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
	
	struct ShaderCreateInfo {
		std::vector<uint32> VertexCode;
		std::vector<uint32> FragmentCode;

		std::vector<vk::VertexInputBindingDescription> VertexInputBindings;

		ShaderReflectionInfo ReflectionInfo;

		// Names of uniform buffers that are shared with other shaders and so aren't auto created
		const inline static std::unordered_set<std::string> SharedUniformBuffers = {
			"v_ubo",
			"f_ubo"
		};

		bool Wireframe = false;		
		bool DepthTesting = true;
	};

	class VulkanShader {
	public:
		VulkanShader(const ShaderCreateInfo& createInfo, VulkanRenderpass& renderpass);
		~VulkanShader();

		void SetUniformBuffer(const std::string& name, const VulkanUniformBuffer& uniformBuffer);
		void SetImageSampler(const std::string& name, vk::Sampler sampler, const std::vector<vk::ImageView>& imageViews);

		template<typename T>
		void UpdateUniformBuffer(const std::string& name, const T& data) {
			std::optional<uint32> binding = _GetBinding(name);
			if (binding)
				m_UniformBuffers.at(*binding)->Update<T>(data);
		}

		void Bind(vk::CommandBuffer commandBuffer);

	private:
		VulkanShader(const VulkanShader&) = delete;
		VulkanShader(VulkanShader&&) = delete;
		VulkanShader& operator=(const VulkanShader&) = delete;
		VulkanShader& operator=(VulkanShader&&) = delete;

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
}