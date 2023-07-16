#pragma once

#include "Core/CoreInclude.hpp"

struct GLFWwindow;

namespace Sphynx::Rendering {
	class SE_API Window {
	public:
		Window(const std::string_view title, uint32_t width, uint32_t height, bool fullscreen = false);
		~Window();

		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;

		void Update();

		bool ShouldClose();

		void SetSize(uint32_t width, uint32_t height);
		void SetWidth(uint32_t width);
		void SetHeight(uint32_t height);
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		void SetMaximized(bool maximized);
		bool IsMaximized() const { return m_Maximized; }

		bool IsMinimized() const { return m_Minimized; }

		bool IsFocused();

		bool IsHovered();

		const glm::vec2& GetPosition() const { return m_Position; }

		void SetIcon(const std::filesystem::path& filepath);

		GLFWwindow* GetGLFWHandle() { return m_Window; }

		void SetResizeCallback(const std::function<void(Window*)>& callback) { m_ResizeCallback = callback; }

	private:
		static void _FramebufferResizedCallback(GLFWwindow* window, int width, int height);
		static void _WindowResizedCallback(GLFWwindow* window, int width, int height);
		static void _WindowPositionCallback(GLFWwindow* window, int xpos, int ypos);
		static void _WindowMaximizeCallback(GLFWwindow* window, int maximized);

	private:
		inline static int s_WindowCount = 0;

		GLFWwindow* m_Window = nullptr;
		std::string m_Title;
		uint32_t m_Width = 0, m_Height = 0;
		glm::vec2 m_Position{ 0, 0 };
		bool m_Maximized = false, m_Minimized = false;

		std::function<void(Window*)> m_ResizeCallback;
	};
}