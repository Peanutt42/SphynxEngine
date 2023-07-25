#pragma once

#include "Core/CoreInclude.hpp"

#include "Vulkan/VulkanShader.hpp"

namespace Sphynx::Rendering {
	// Difference to VulkanShader: 
	//  Shader is more for game level shaders,
	//  VulkanShader more for engine internal shaders
	class SE_API Shader {
	public:
		Shader(const std::filesystem::path& filepath);

		void Bind();

		void UploadToGPU();

	private:
		std::unique_ptr<VulkanShader> m_VulkanShader;
		ShaderCreateInfo m_CreateInfo;
	};
}