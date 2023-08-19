#pragma once

#include "pch.hpp"
#include "Window.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Scene/Scene.hpp"

namespace Sphynx::Rendering {
	class SE_API Renderer {
	public:
		Renderer(Window& window, const std::function<void()>& resizeCallback);
		~Renderer();

		void SubmitScene(Scene& scene, const Camera& camera);

		void Begin();
		void End();

		void WaitBeforeClose();

		// ImTextureID
		void* GetSceneTextureID();

	private:
		Window& m_Window;
		std::unique_ptr<Mesh> m_CubeMesh;
		std::unique_ptr<Shader> m_DefaultShader;

		bool m_GeneratedSceneTextureDescriptorSets = false;

		struct RenderCommand {
			std::vector<Transform> Transforms;
			Camera Camera;
		} m_RenderCommand;
	};
}