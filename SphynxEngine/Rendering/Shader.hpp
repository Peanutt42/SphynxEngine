#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class VulkanShader;

	// Difference to VulkanShader: 
	//  Shader is more for game level shaders,
	//  VulkanShader more for engine internal shaders
	class SE_API Shader {
	public:
		Shader(BufferView vertexCode, BufferView fragmentCode, size_t vertexBufferElementSize, size_t instanceBufferElementSize = 0);
		~Shader();

		void Bind();

		void UploadToGPU();

		VulkanShader* GetVulkanShader() { return m_VulkanShader; }

	private:
		Shader(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&) = delete;

	private:
		VulkanShader* m_VulkanShader = nullptr;
		size_t m_VertexBufferElementSize = 0, m_InstanceBufferElementSize = 0;
		std::vector<uint32> m_VertexSpirv, m_FragmentSpirv;
	};
}