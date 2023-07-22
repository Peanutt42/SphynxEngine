#pragma once

#include "Core/CoreInclude.hpp"
#include "Window.hpp"

#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	class SE_API Renderer {
	public:
		Renderer(Window& window, const std::function<void()>& resizeCallback);
		~Renderer();

		void Begin();
		void End();

		void WaitBeforeClose();

	private:
		Window& m_Window;
	};
}