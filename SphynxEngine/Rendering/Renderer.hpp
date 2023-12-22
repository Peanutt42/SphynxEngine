#pragma once

#include "pch.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Scene/Scene.hpp"

#include <queue>

namespace Sphynx::Rendering {
	class SE_API Renderer {
	public:
		static bool Init(Window& window, const std::function<void()>& resizeCallback, bool vsync);
		static void Shutdown();

		static void SubmitScene(Scene& scene, const Camera& camera);
		static void SubmitBillboard(const glm::vec3& position, uint32 textureID);
		static void SubmitLine(const glm::vec3& start, const glm::vec3& end);

		static void Update();

		static uint32 GetSceneTextureID();

		static float GetSceneAspectRatio();

		static bool IsInitialized();
	};
}