#include "pch.hpp"
#include "Renderer.hpp"

#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	Renderer::Renderer(Window& window, const std::function<void()>& resizeCallback)
		: m_Window(window)
	{
		SE_PROFILE_FUNCTION();

		m_Window.SetResizeCallback([this, resizeCallback](Window* window) {
			if (window->GetWidth() != 0 && window->GetHeight() != 0 && resizeCallback) {

				VulkanContext::FramebufferResized = true;
				resizeCallback();
			}
		});

		VulkanContext::Init(m_Window);

		MeshData data;
		data.LoadMesh("Engine/Resources/Meshes/cube.semesh");
		m_CubeMesh = std::make_unique<Mesh>(data);

		m_DefaultShader = std::make_unique<Shader>("Engine/Resources/Shaders/Default.glsl");
		m_DefaultShader->UploadToGPU();
	}

	Renderer::~Renderer() {
		SE_PROFILE_FUNCTION();

		m_DefaultShader.reset();
		m_CubeMesh.reset();
		VulkanContext::Shutdown();
	}

	void Renderer::Begin() {
		SE_PROFILE_FUNCTION();

		VulkanContext::BeginSceneRenderpass();
		// Draw Scene
		m_DefaultShader->Bind();
		m_CubeMesh->Draw(1);
		VulkanContext::EndSceneRenderpass();
		VulkanContext::BeginLastRenderpass();
	}

	void Renderer::End() {
		SE_PROFILE_FUNCTION();

		VulkanContext::EndLastRenderpass();

		VulkanContext::Submit();
	}

	void Renderer::WaitBeforeClose() {
		VulkanContext::WaitBeforeClose();
	}
}