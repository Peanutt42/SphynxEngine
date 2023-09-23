#include "pch.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	Shader::Shader(BufferView vertexCode, BufferView fragmentCode) {
		m_VertexSpirv.resize(vertexCode.GetSize<uint32>());
		std::memcpy(m_VertexSpirv.data(), vertexCode.As<uint32>(), vertexCode.Size);

		m_FragmentSpirv.resize(fragmentCode.GetSize<uint32>());
		std::memcpy(m_FragmentSpirv.data(), fragmentCode.As<uint32>(), fragmentCode.Size);
	}

	Shader::~Shader() {
		delete m_VulkanShader;
	}

	void Shader::Bind() {
		m_VulkanShader->Bind(VulkanContext::CommandBuffer);
	}
	
	void Shader::UploadToGPU() {
		ShaderCreateInfo info{
			.VertexCode = std::move(m_VertexSpirv),
			.FragmentCode = std::move(m_FragmentSpirv),
			.VertexInputBindings = std::vector<vk::VertexInputBindingDescription>{
				vk::VertexInputBindingDescription{ 0, sizeof(Vertex), vk::VertexInputRate::eVertex },
				vk::VertexInputBindingDescription{ 1, sizeof(InstanceData), vk::VertexInputRate::eInstance }
			}
		};

		SpirvHelper::GetReflectionInfo(info.VertexCode, vk::ShaderStageFlagBits::eVertex, info.ReflectionInfo);
		SpirvHelper::GetReflectionInfo(info.FragmentCode, vk::ShaderStageFlagBits::eFragment, info.ReflectionInfo);
		
		std::sort(info.ReflectionInfo.VertexAttributes.begin(), info.ReflectionInfo.VertexAttributes.end(),
			[](const vk::VertexInputAttributeDescription& l, const vk::VertexInputAttributeDescription& r) {
				return l.location < r.location;
			});

		// Find the first location of the instance attributes
		uint32 firstInstanceLocation = std::numeric_limits<uint32>::max();
		uint32 instanceAttributesSize = 0;
		for (int i = (int)info.ReflectionInfo.VertexAttributes.size() - 1; i >= 0; i--) {
			auto& attribute = info.ReflectionInfo.VertexAttributes[i];

			instanceAttributesSize += info.ReflectionInfo.VertexInputAttributeSizes.at(attribute.location);

			if (sizeof(InstanceData) == instanceAttributesSize) {
				firstInstanceLocation = attribute.location;
				break;
			}
		}

		// Set all the attribute's bindings and offsets
		uint32 vertexAttributesSize = 0;
		instanceAttributesSize = 0;
		for (auto& attribute : info.ReflectionInfo.VertexAttributes) {
			if (attribute.location < firstInstanceLocation) {
				attribute.binding = 0;
				attribute.offset = vertexAttributesSize;
				vertexAttributesSize += info.ReflectionInfo.VertexInputAttributeSizes.at(attribute.location);
			}
			else {
				attribute.binding = 1;
				attribute.offset = instanceAttributesSize;
				instanceAttributesSize += info.ReflectionInfo.VertexInputAttributeSizes.at(attribute.location);
			}
		}

		m_VulkanShader = new VulkanShader(info, *VulkanContext::SceneRenderpass);
	}
}