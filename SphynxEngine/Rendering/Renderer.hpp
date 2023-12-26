#pragma once

#include "pch.hpp"
#include "Window.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Scene/Scene.hpp"

#include <queue>

namespace Sphynx::Rendering {
	class SE_API Renderer {
	public:
		static bool Init(Window& window, const std::function<void()>& resizeCallback);
		static void Shutdown();

		static void SubmitScene(Scene& scene, const Camera& camera);
		static void SubmitBillboard(const glm::vec3& position);

		static void Begin();
		static void End();

		static void WaitBeforeClose();

		static void AddBeforeNextRenderCallback(const std::function<void()>& callback);

		static void SetDrawSceneTextureEnabled(bool enable);

		// ImTextureID
		static void* GetSceneTextureID();

		static bool IsInitialized();
	};
}