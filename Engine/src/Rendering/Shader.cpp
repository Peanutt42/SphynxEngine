#include "pch.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	VertexInput GetVertexInputDescription() {
		VertexInput input;
		input.Description.binding = 0;
		input.Description.stride = sizeof(Vertex);
		input.Description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VulkanVertexAttributeBuilder builder;
		builder.Add(AttributeFormat::Float3, offsetof(Vertex, Position));
		builder.Add(AttributeFormat::Float3, offsetof(Vertex, Normal));
		builder.Add(AttributeFormat::Float2, offsetof(Vertex, UV));
		// Note: set the binding to 1 for per instance data

		input.Attributes = builder.GetAttributes();

		return input;
	}

	Shader::Shader(BufferView vertexCode, BufferView fragmentCode) {
		m_VertexSpirv.resize(vertexCode.Size / sizeof(uint32_t));
		std::memcpy(m_VertexSpirv.data(), vertexCode.As<uint32_t>(), vertexCode.Size);

		m_FragmentSpirv.resize(fragmentCode.Size / sizeof(uint32_t));
		std::memcpy(m_FragmentSpirv.data(), fragmentCode.As<uint32_t>(), fragmentCode.Size);
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
			.VertexInput = GetVertexInputDescription()
		};
		m_VulkanShader = new VulkanShader(info, *VulkanContext::SceneRenderpass);
	}
}