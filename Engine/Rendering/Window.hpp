#pragma once

#include "pch.hpp"

struct GLFWwindow;

namespace Sphynx::Rendering {
	class Window {
	public:
		Window(const std::string_view title, bool maximized = true, bool fullscreen = false, bool customWindowControls = true);
		~Window();

		void Update();

		bool ShouldClose();

		void SetSize(uint32 width, uint32 height);
		void SetWidth(uint32 width);
		void SetHeight(uint32 height);
		uint32 GetWidth() const { return m_Width; }
		uint32 GetHeight() const { return m_Height; }
		const std::string& GetTitle() const { return m_Title; }

		void Maximize();
		void Restore();
		bool IsMaximized() const { return m_Maximized; }

		void Minimize();
		bool IsMinimized() const { return m_Minimized; }

		bool IsFocused() const { return m_Focused; }

		bool IsHovered() const { return m_Hovered; }

		bool IsFullscreen() const { return m_Fullscreen; }

		const glm::vec2& GetPosition() const { return m_Position; }

		glm::ivec2 GetFramebufferSize();

		void SetIcon(const std::filesystem::path& filepath);

		GLFWwindow* GetGLFWHandle() { return m_Window; }

		void SetResizeCallback(const std::function<void(Window*)>& callback) { m_ResizeCallback = callback; }
		void SetResizeCallbackEnable(bool enabled) { m_EnableResizeCallback = enabled; }

		void SetTitlebarhitTestCallback(const std::function<bool()>& callback) { m_TitlebarhitTest = callback; }

	private:
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

		static void _FramebufferResizedCallback(GLFWwindow* window, int width, int height);
		static void _WindowResizedCallback(GLFWwindow* window, int width, int height);
		static void _WindowPositionCallback(GLFWwindow* window, int xpos, int ypos);
		static void _WindowMaximizeCallback(GLFWwindow* window, int maximized);
		static void _TitlebarHitTestCallback(GLFWwindow* window, int x, int y, int* hit);

	private:
		inline static int s_WindowCount = 0;

		GLFWwindow* m_Window = nullptr;
		std::string m_Title;
		uint32 m_Width = 0, m_Height = 0;
		glm::vec2 m_Position{ 0, 0 };

		bool m_Maximized = false;
		bool m_Minimized = false;
		bool m_Hovered = false;
		bool m_Focused = false;
		bool m_Fullscreen = false;

		// Stores all changes for the window for the next Update() call from the main thread
		std::vector<std::function<void()>> m_PendingMainThreadCallbacks;

		std::function<void(Window*)> m_ResizeCallback;
		bool m_EnableResizeCallback = true;
		std::function<bool()> m_TitlebarhitTest;
	};
}