#include "pch.hpp"
#include "Renderer.hpp"
#include "Image.hpp"
#include "Scene/AllComponents.hpp"
#include "Profiling/Profiling.hpp"

#include "Vulkan/VulkanContext.hpp"

uint32_t g_DefaultVertex[] = {
#include "../../Content/Shaders/Embedded/Default.vert.embed"
};

uint32_t g_DefaultFragment[] = {
#include "../../Content/Shaders/Embedded/Default.frag.embed"
};

uint32_t g_BillboardVertex[] = {
#include "../../Content/Shaders/Embedded/Billboard.vert.embed"
};

uint32_t g_BillboardFragment[] = {
#include "../../Content/Shaders/Embedded/Billboard.frag.embed"
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
	struct InstanceData {
		glm::vec3 Albedo;
		float Metalic;
		float Roughness;
		glm::mat4 Model;
	};
	inline static VulkanInstanceBuffer* s_InstanceBuffer = nullptr;

	Shader* s_BillboardShader = nullptr;
	struct BillboardInstanceData {
		glm::vec3 Position;
		glm::vec3 Color;
		float TexIndex = 0.f;
	};
	VulkanInstanceBuffer* s_BillboardInstanceBuffer = nullptr;

	std::queue<std::function<void()>> s_BeforeNextRenderCallbacks;

	struct CameraUniformBufferData {
		glm::mat4 Projection;
		glm::mat4 View;
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
		std::vector<BillboardInstanceData> Billboards;
		std::vector<VulkanTexture*> BillboardTextures;
		Camera SceneCamera;
		LightsUniformBufferData LightsData;
	};
	RenderCommand s_RenderCommand;

	Image* s_ErrorImage = nullptr;

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

		s_ErrorImage = new Image((std::filesystem::path)"Content/Textures/Error.png");

		CameraUniformBuffer = new VulkanUniformBuffer(sizeof(CameraUniformBufferData));
		LightsUniformBuffer = new VulkanUniformBuffer(sizeof(LightsUniformBufferData));

		MeshData data;
		data.LoadMesh("Content/Meshes/cube.semesh");
		s_CubeMesh = new Mesh(data);

		s_DefaultShader = new Shader(BufferView(g_DefaultVertex), BufferView(g_DefaultFragment), sizeof(Vertex), sizeof(InstanceData));
		s_DefaultShader->UploadToGPU();
		s_DefaultShader->GetVulkanShader()->SetupUniformBuffer("CameraData", *CameraUniformBuffer);
		s_DefaultShader->GetVulkanShader()->SetupUniformBuffer("LightData", *LightsUniformBuffer);
		s_InstanceBuffer = new VulkanInstanceBuffer(sizeof(InstanceData));

		s_BillboardShader = new Shader(g_BillboardVertex, g_BillboardFragment, 0, sizeof(BillboardInstanceData));
		s_BillboardShader->UploadToGPU();
		s_BillboardShader->GetVulkanShader()->SetupUniformBuffer("CameraData", *CameraUniformBuffer);
		for (int i = 0; i < 32; i++)
			s_BillboardShader->GetVulkanShader()->SetupImageSampler("image" + std::to_string(i), VulkanContext::DefaultSampler, s_ErrorImage->GetVulkanTexture()->GetImageViews());
		
		s_BillboardInstanceBuffer = new VulkanInstanceBuffer(sizeof(BillboardInstanceData));

		s_ScreenQuadShader = new Shader(BufferView(g_ScreenQuadVertex), BufferView(g_ScreenQuadFragment), sizeof(Vertex));
		s_ScreenQuadShader->UploadToGPU();
		s_ScreenQuadShader->GetVulkanShader()->SetupImageSampler("screen", VulkanContext::DefaultSampler, VulkanContext::SceneRenderpass->GetImageViews(0/*color*/));

		s_Initialized = true;
		return true;
	}

	void Renderer::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		delete s_InstanceBuffer;
		delete s_BillboardInstanceBuffer;

		delete s_ScreenQuadShader;
		delete s_BillboardShader;
		delete s_DefaultShader;
		delete s_CubeMesh;
		delete LightsUniformBuffer;
		delete CameraUniformBuffer;
		
		delete s_ErrorImage;

		VulkanContext::Shutdown();

		s_Initialized = false;
	}

	void Renderer::SubmitScene(Scene& scene, const Camera& camera) {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

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

	void Renderer::SubmitBillboard(const glm::vec3& position, const glm::vec3& color, Image& image) {
		VulkanTexture* texture = image.GetVulkanTexture();
		int textureIndex = 0;
		bool foundTexture = false;
		for (int i = 0; i < s_RenderCommand.BillboardTextures.size(); i++) {
			if (s_RenderCommand.BillboardTextures[i] == texture) {
				textureIndex = i;
				foundTexture = true;
				break;
			}
		}
		if (!foundTexture) {
			s_RenderCommand.BillboardTextures.push_back(texture);
			textureIndex = s_RenderCommand.BillboardTextures.size() - 1;
		}
		s_RenderCommand.Billboards.emplace_back(position, color, (float)textureIndex);
	}

	void Renderer::Begin() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		if (s_RenderCommand.ModelMatrices.size() > s_InstanceBuffer->GetSize())
			s_InstanceBuffer->Resize(s_RenderCommand.ModelMatrices.size() * 2);

		if (s_RenderCommand.Billboards.size() > s_BillboardInstanceBuffer->GetSize())
			s_BillboardInstanceBuffer->Resize(s_RenderCommand.Billboards.size() * 2);

		while (!s_BeforeNextRenderCallbacks.empty()) {
			s_BeforeNextRenderCallbacks.front()();
			s_BeforeNextRenderCallbacks.pop();
		}


		float aspect = GetAspect((float)VulkanContext::SceneWidth, (float)VulkanContext::SceneHeight);
		CameraUniformBufferData cameraUniformBufferData{
			.Projection = s_RenderCommand.SceneCamera.GetPerspective(aspect),
			.View = s_RenderCommand.SceneCamera.GetView(),
			.CameraPosition = s_RenderCommand.SceneCamera.Position,
		};
		CameraUniformBuffer->Update(cameraUniformBufferData);

		LightsUniformBuffer->Update(s_RenderCommand.LightsData);

		s_InstanceBuffer->Set(s_RenderCommand.ModelMatrices);

		s_BillboardInstanceBuffer->Set(s_RenderCommand.Billboards);
		
		VulkanContext::Begin();

		// Billboards
		if (s_RenderCommand.BillboardTextures.size() > 32)
			SE_WARN(Logging::Rendering, "Too many different billboard textures (>32). Modify the code and the shader!");
		for (size_t i = 0; i < s_RenderCommand.BillboardTextures.size(); i++) {
			s_BillboardShader->GetVulkanShader()->UpdateImageSampler("image" + std::to_string(i), VulkanContext::DefaultSampler, s_RenderCommand.BillboardTextures[i]->GetImageViews());
		}

		VulkanContext::StartRecording();

		VulkanContext::BeginSceneRenderpass();
		// Draw Scene
		{
			// Cubes
			s_DefaultShader->Bind();
			s_InstanceBuffer->Bind();
			s_CubeMesh->Draw((uint32)s_RenderCommand.ModelMatrices.size());
			s_RenderCommand.ModelMatrices.resize(0);
			
			// Billboards
			if (!s_RenderCommand.Billboards.empty()) {
				s_BillboardShader->Bind();
				s_BillboardInstanceBuffer->Bind();
				VulkanContext::CommandBuffer.draw(6, s_RenderCommand.Billboards.size(), 0, 0);
			}
			s_RenderCommand.Billboards.resize(0);
			s_RenderCommand.BillboardTextures.resize(0);
		}
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

		VulkanContext::StopRecording();

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