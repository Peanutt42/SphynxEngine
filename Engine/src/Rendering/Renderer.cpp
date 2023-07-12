#include "pch.hpp"
#include "Renderer.hpp"

namespace Sphynx::Rendering {
	Renderer::Renderer(Window& window, const std::function<void()>& resizeCallback)
		: m_Window(window), m_ResizeCallback(resizeCallback)
	{
		SE_PROFILE_FUNCTION();

		m_Window.SetResizeCallback([this](Window*) {
			//m_Instance->FramebufferResized = true;
			if (m_ResizeCallback)
				m_ResizeCallback();
		});

		m_Context = std::make_unique<VulkanContext>(m_Window);
	}

	Renderer::~Renderer() {
		SE_PROFILE_FUNCTION();

		m_Context.reset();
	}

	void Renderer::Update() {
		m_Context->Update();
	}

	void Renderer::WaitBeforeClose() {
		m_Context->WaitBeforeClose();
	}
}