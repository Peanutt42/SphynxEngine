#include "pch.hpp"
#include "Renderer.hpp"
#include "Mesh.hpp"

namespace Sphynx::Rendering {
	Mesh* cubeMesh = nullptr;

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
	}

	Renderer::~Renderer() {
		SE_PROFILE_FUNCTION();

		VulkanContext::Shutdown();
	}

	void Renderer::Begin() {
		VulkanContext::BeginSceneRenderpass();
		// Draw Scene
		VulkanContext::EndSceneRenderpass();
		VulkanContext::BeginLastRenderpass();
	}

	void Renderer::End() {
		VulkanContext::EndLastRenderpass();
	}

	void Renderer::WaitBeforeClose() {
		VulkanContext::WaitBeforeClose();
	}
}