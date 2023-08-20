#include "pch.hpp"
#include "Renderer.hpp"
#include "Scene/DefaultComponents.hpp"

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
		m_DefaultShader->GetVulkanShader()->SetUniformBuffer("v_ubo", *VulkanContext::UniformBuffer);
	}

	Renderer::~Renderer() {
		SE_PROFILE_FUNCTION();

		m_DefaultShader.reset();
		m_CubeMesh.reset();
		VulkanContext::Shutdown();
	}

	void Renderer::SubmitScene(Scene& scene, const Camera& camera) {
		m_RenderCommand.ModelMatrices.clear();
		m_RenderCommand.Camera = camera;

		// TODO: actual impl.
		auto view = scene.View<ECS::TransformComponent>();
		view.ForEach([&](ECS::EntityId entity, const ECS::TransformComponent& transform) {
			m_RenderCommand.ModelMatrices.emplace_back(transform.GetModelMatrix());
		});
	}

	void Renderer::Begin() {
		SE_PROFILE_FUNCTION();

		if (m_RenderCommand.ModelMatrices.size() > VulkanContext::InstanceBuffer->GetSize())
			VulkanContext::InstanceBuffer->Resize(m_RenderCommand.ModelMatrices.size() * 2);

		while (!m_BeforeNextRenderCallbacks.empty()) {
			m_BeforeNextRenderCallbacks.front()();
			m_BeforeNextRenderCallbacks.pop();
		}


		float aspect = GetAspect((float)VulkanContext::SceneWidth, (float)VulkanContext::SceneHeight);
		UniformBufferData uniformBufferData{
			.proj_view = m_RenderCommand.Camera.GetPerspective(aspect) * m_RenderCommand.Camera.GetView()
		};
		VulkanContext::UniformBuffer->Update(uniformBufferData);

		VulkanContext::InstanceBuffer->Set(m_RenderCommand.ModelMatrices);

		VulkanContext::BeginSceneRenderpass();
		// Draw Scene
		m_DefaultShader->Bind();
		VulkanContext::InstanceBuffer->Bind();
		m_CubeMesh->Draw((uint32)m_RenderCommand.ModelMatrices.size());
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