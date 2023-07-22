#include "pch.hpp"
#include "Renderer.hpp"

namespace Sphynx::Rendering {
	Renderer::Renderer(Window& window, const std::function<void()>& resizeCallback)
		: m_Window(window), m_ResizeCallback(resizeCallback)
	{
		SE_PROFILE_FUNCTION();

		m_Window.SetResizeCallback([this](Window*) {
			VulkanContext::FramebufferResized = true;
			if (m_ResizeCallback)
				m_ResizeCallback();
		});

		VulkanContext::Init(m_Window);
	}

	Renderer::~Renderer() {
		SE_PROFILE_FUNCTION();

		VulkanContext::Shutdown();
	}

	void Renderer::Begin() {
		VulkanContext::Begin();
	}

	void Renderer::End() {
		VulkanContext::End();
	}

	void Renderer::WaitBeforeClose() {
		VulkanContext::WaitBeforeClose();
	}
}