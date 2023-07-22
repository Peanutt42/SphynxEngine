#include "pch.hpp"
#include "Renderer.hpp"

namespace Sphynx::Rendering {
	Renderer::Renderer(Window& window, const std::function<void()>& resizeCallback)
		: m_Window(window)
	{
		SE_PROFILE_FUNCTION();

		m_Window.SetResizeCallback([this, resizeCallback](Window* window) {
			if (window->GetWidth() != 0 && window->GetHeight() != 0 && resizeCallback) {

				VulkanContext::FramebufferResized = true;
				resizeCallback();
			}
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