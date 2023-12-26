#include "pch.hpp"
#include "Renderer.hpp"
#include "Scene/AllComponents.hpp"
#include "Profiling/Profiling.hpp"

#include "Vulkan/VulkanContext.hpp"

uint32_t g_DefaultVertex[] = {
#include "../../Content/Shaders/Embedded/Default.vert.embed"
};

uint32_t g_DefaultFragment[] = {
#include "../../Content/Shaders/Embedded/Default.frag.embed"
};

uint32_t g_ScreenQuadVertex[] = {
#include "../../Content/Shaders/Embedded/ScreenQuad.vert.embed"
};

uint32_t g_ScreenQuadFragment[] = {
#include "../../Content/Shaders/Embedded/ScreenQuad.frag.embed"
};

namespace Sphynx::Rendering {
	bool s_Initialized = false;

	Mesh* s_CubeMesh = nullptr;
	Shader* s_DefaultShader = nullptr;
	Shader* s_ScreenQuadShader = nullptr;

	std::queue<std::function<void()>> s_BeforeNextRenderCallbacks;

	struct CameraUniformBufferData {
		glm::mat4 ProjView; // proj * view
		glm::vec3 CameraPosition;
	};
	inline static VulkanUniformBuffer* CameraUniformBuffer = nullptr;
	struct LightsUniformBufferData {
		struct Light {
			alignas(16) glm::vec3 Position{ 0 };
			alignas(16) glm::vec3 Color{ 0 };
		};

		std::array<Light, 4> Lights;
	};
	inline static VulkanUniformBuffer* LightsUniformBuffer = nullptr;

	bool s_DrawSceneTexture = false;
	bool s_GeneratedSceneTextureDescriptorSets = false;

	struct RenderCommand {
		std::vector<InstanceData> ModelMatrices;
		Camera SceneCamera;
		LightsUniformBufferData LightsData;
	};
	RenderCommand s_RenderCommand;

	bool Renderer::Init(Window& window, const std::function<void()>& resizeCallback) {
		SE_PROFILE_FUNCTION();

		if (s_Initialized)
			return true;

		window.SetResizeCallback([resizeCallback](Window* window, int width, int height) {
			if (width != 0 && height != 0) {
				if (VulkanContext::SwapChain && VulkanContext::Renderpass)
					VulkanContext::SwapChain->Recreate(VulkanContext::Renderpass->GetHandle());
				if (resizeCallback)
					resizeCallback();
			}
		});

		VulkanContext::Init(window);

		CameraUniformBuffer = new VulkanUniformBuffer(sizeof(CameraUniformBufferData));
		LightsUniformBuffer = new VulkanUniformBuffer(sizeof(LightsUniformBufferData));

		MeshData data;
		data.LoadMesh("Content/Meshes/cube.semesh");
		s_CubeMesh = new Mesh(data);

		s_DefaultShader = new Shader(BufferView(g_DefaultVertex), BufferView(g_DefaultFragment));
		s_DefaultShader->UploadToGPU();
		s_DefaultShader->GetVulkanShader()->SetUniformBuffer("CameraData", *CameraUniformBuffer);
		s_DefaultShader->GetVulkanShader()->SetUniformBuffer("LightData", *LightsUniformBuffer);

		s_ScreenQuadShader = new Shader(BufferView(g_ScreenQuadVertex), BufferView(g_ScreenQuadFragment));
		s_ScreenQuadShader->UploadToGPU();
		s_ScreenQuadShader->GetVulkanShader()->SetImageSampler("screen", VulkanContext::DefaultSampler, VulkanContext::SceneRenderpass->GetImageViews(0/*color*/));

		s_Initialized = true;
		return true;
	}

	void Renderer::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		delete s_ScreenQuadShader;
		delete s_DefaultShader;
		delete s_CubeMesh;
		delete LightsUniformBuffer;
		delete CameraUniformBuffer;

		VulkanContext::Shutdown();

		s_Initialized = false;
	}

	void Renderer::SubmitScene(Scene& scene, const Camera& camera) {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		s_RenderCommand.ModelMatrices.clear();
		s_RenderCommand.SceneCamera = camera;

		for (auto[entity, transform, mesh] : scene.View<ECS::TransformComponent, MeshComponent>().each()) {
			s_RenderCommand.ModelMatrices.push_back(InstanceData{ mesh.Albedo, mesh.Metalic, mesh.Roughness, transform.GetModelMatrix()});
		}

		int i = 0;
		for (auto [entity, transform, light] : scene.View<ECS::TransformComponent, LightComponent>().each()) {
			s_RenderCommand.LightsData.Lights[i] = {
				transform.Position,
				light.Color
			};
			i++;
		}
	}

	void Renderer::Begin() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		if (s_RenderCommand.ModelMatrices.size() > VulkanContext::InstanceBuffer->GetSize())
			VulkanContext::InstanceBuffer->Resize(s_RenderCommand.ModelMatrices.size() * 2);

		while (!s_BeforeNextRenderCallbacks.empty()) {
			s_BeforeNextRenderCallbacks.front()();
			s_BeforeNextRenderCallbacks.pop();
		}


		float aspect = GetAspect((float)VulkanContext::SceneWidth, (float)VulkanContext::SceneHeight);
		CameraUniformBufferData cameraUniformBufferData{
			.ProjView = s_RenderCommand.SceneCamera.GetPerspective(aspect) * s_RenderCommand.SceneCamera.GetView(),
			.CameraPosition = s_RenderCommand.SceneCamera.Position,
		};
		CameraUniformBuffer->Update(cameraUniformBufferData);

		LightsUniformBuffer->Update(s_RenderCommand.LightsData);

		VulkanContext::InstanceBuffer->Set(s_RenderCommand.ModelMatrices);

		VulkanContext::Begin();

		VulkanContext::BeginSceneRenderpass();
		// Draw Scene
		s_DefaultShader->Bind();
		VulkanContext::InstanceBuffer->Bind();
		s_CubeMesh->Draw((uint32)s_RenderCommand.ModelMatrices.size());
		VulkanContext::EndSceneRenderpass();

		VulkanContext::BeginLastRenderpass();
	}

	void Renderer::End() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		if (s_DrawSceneTexture) {
			s_ScreenQuadShader->Bind();
			VulkanContext::CommandBuffer.draw(6, 1, 0, 0);
		}

		VulkanContext::EndLastRenderpass();

		VulkanContext::Finish();
	}

	void Renderer::WaitBeforeClose() {
		if (!s_Initialized)
			return;

		VulkanContext::WaitBeforeClose();
	}

	void Renderer::AddBeforeNextRenderCallback(const std::function<void()>& callback) { s_BeforeNextRenderCallbacks.push(callback); }

	void Renderer::SetDrawSceneTextureEnabled(bool enable) { s_DrawSceneTexture = enable; }

	void* Renderer::GetSceneTextureID() {
		if (!s_Initialized)
			return nullptr;

		if (!s_GeneratedSceneTextureDescriptorSets) {
			VulkanContext::GenerateSceneTextureDescriptorSets();
			s_GeneratedSceneTextureDescriptorSets = true;
		}
		return (void*)VulkanContext::SceneTextureDescriptorSets[VulkanContext::CurrentImage];
	}

	bool Renderer::IsInitialized() { return s_Initialized; }
}