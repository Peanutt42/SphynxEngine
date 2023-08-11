#include "pch.hpp"
#include "Renderer.hpp"

#include "Vulkan/VulkanContext.hpp"

uint32_t g_DefaultVertex[] = {
#include "../Resources/Shaders/Embedded/Default.vert.embed"
};

uint32_t g_DefaultFragment[] = {
#include "../Resources/Shaders/Embedded/Default.frag.embed"
};

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

		m_DefaultShader = std::make_unique<Shader>(BufferView(g_DefaultVertex), BufferView(g_DefaultFragment));
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

		VulkanContext::Finish();
	}

	void Renderer::WaitBeforeClose() {
		VulkanContext::WaitBeforeClose();
	}

	void* Renderer::GetSceneTextureID() {
		if (!m_GeneratedSceneTextureDescriptorSets) {
			VulkanContext::GenerateSceneTextureDescriptorSets();
			m_GeneratedSceneTextureDescriptorSets = true;
		}
		return (void*)VulkanContext::SceneTextureDescriptorSets[VulkanContext::CurrentImage];
	}
}