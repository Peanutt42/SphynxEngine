#include "pch.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	Shader::Shader(BufferView vertexCode, BufferView fragmentCode) {
		m_CreateInfo.VertexCode.resize(vertexCode.Size / sizeof(uint32_t));
		std::memcpy(m_CreateInfo.VertexCode.data(), vertexCode.As<uint32_t>(), vertexCode.Size);

		m_CreateInfo.FragmentCode.resize(fragmentCode.Size / sizeof(uint32_t));
		std::memcpy(m_CreateInfo.FragmentCode.data(), fragmentCode.As<uint32_t>(), fragmentCode.Size);
		
		m_CreateInfo.VertexInput = Vertex::GetInputDescription();
	}

	void Shader::Bind() {
		m_VulkanShader->Bind(VulkanContext::CommandBuffer);
	}
	
	void Shader::UploadToGPU() {
		m_VulkanShader = std::make_unique<VulkanShader>(m_CreateInfo, *VulkanContext::SceneRenderpass);
	}
}