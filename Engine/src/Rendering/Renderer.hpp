#pragma once

#include "pch.hpp"
#include "Window.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "InstanceData.hpp"
#include "Scene/Scene.hpp"

#include <queue>

namespace Sphynx::Rendering {
	class SE_API Renderer {
	public:
		Renderer(Window& window, const std::function<void()>& resizeCallback);
		~Renderer();

		void SubmitScene(Scene& scene, const Camera& camera);

		void Begin();
		void End();

		void WaitBeforeClose();

		void AddBeforeNextRenderCallback(const std::function<void()>& callback) { m_BeforeNextRenderCallbacks.push(callback); }

		void SetDrawSceneTextureEnabled(bool enable) { m_DrawSceneTexture = enable; }

		// ImTextureID
		void* GetSceneTextureID();

	private:
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;

	private:
		Window& m_Window;
		std::unique_ptr<Mesh> m_CubeMesh;
		std::unique_ptr<Shader> m_DefaultShader;
		std::unique_ptr<Shader> m_ScreenQuadShader;

		std::queue<std::function<void()>> m_BeforeNextRenderCallbacks;

		bool m_DrawSceneTexture = false;
		bool m_GeneratedSceneTextureDescriptorSets = false;

		struct RenderCommand {
			std::vector<InstanceData> ModelMatrices;
			Camera Camera;
		} m_RenderCommand;
	};
}