#include "pch.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	Shader::Shader(const std::filesystem::path& filepath)
		: m_CreateInfo(filepath)
	{
		m_CreateInfo.VertexInput = Vertex::GetInputDescription();
	}

	void Shader::Bind() {
		m_VulkanShader->Bind(VulkanContext::CommandBuffer);
	}
	
	void Shader::UploadToGPU() {
		m_VulkanShader = std::make_unique<VulkanShader>(m_CreateInfo, *VulkanContext::SceneRenderpass);
	}
}