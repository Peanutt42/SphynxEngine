#pragma once

#include "pch.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Scene/Scene.hpp"

#include <queue>

namespace Sphynx::Rendering {
	class SE_API Renderer {
	public:
		static bool Init(Window& window, const std::function<void()>& resizeCallback);
		static void Shutdown();

		static void SubmitScene(Scene& scene, const Camera& camera);

		static void Update();

		static uint32 GetSceneTextureID();

		static bool IsInitialized();
	};
}