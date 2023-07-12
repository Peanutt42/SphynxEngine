#pragma once

#include "Core/CoreInclude.hpp"
#include "Window.hpp"

#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	class Renderer {
	public:
		Renderer(Window& window, const std::function<void()>& resizeCallback);
		~Renderer();

		void Update();

		void WaitBeforeClose();

	private:
		Window& m_Window;
		std::function<void()> m_ResizeCallback;

		std::unique_ptr<VulkanContext> m_Context;
	};
}