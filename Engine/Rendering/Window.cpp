#include "pch.hpp"
#include "Window.hpp"
#include "Profiling/Profiling.hpp"

#include <GLFW/glfw3.h>

#include <stb_image.h>
#include <Core/Engine.hpp>

namespace Sphynx::Rendering {
	void GLFWErrorCallback([[maybe_unused]] int error, [[maybe_unused]] const char* description) {
		SE_ERR(Logging::Rendering, "[GLFW]: ({}): {}", error, description);
	}

	Window::Window(const std::string_view title, bool maximized, bool fullscreen, bool customWindowControls)
		: m_Title(title), m_Maximized(maximized)
	{
		SE_PROFILE_FUNCTION();

		if (s_WindowCount == 0) {
			SE_PROFILE_SCOPE("InitGLFW");
			SE_ASSERT(glfwInit(), Logging::Rendering, "Failed to initialize GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
		}
		s_WindowCount++;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_MAXIMIZED, m_Maximized);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// ImGui implements custom titlebar
		if (Engine::GetSettings().ImGuiEnabled && !fullscreen)
			glfwWindowHint(GLFW_TITLEBAR, !customWindowControls);

		GLFWmonitor* monitor = nullptr;
		if (fullscreen) {
			monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
			m_Width = videoMode->width;
			m_Height = videoMode->height;
		}
		else {
			GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* primaryVideoMode = glfwGetVideoMode(primaryMonitor);
			// if maximized, width and height don't matter till the window is restored again
			m_Width = primaryVideoMode->width / 2;
			m_Height = primaryVideoMode->height / 2;
		}

		m_Window = glfwCreateWindow((int)m_Width, (int)m_Height, m_Title.c_str(), monitor, nullptr);
		SE_ASSERT(m_Window, Logging::Rendering, "Failed to create window");

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, _FramebufferResizedCallback);
		glfwSetWindowSizeCallback(m_Window, _WindowResizedCallback);
		glfwSetWindowPosCallback(m_Window, _WindowPositionCallback);
		glfwSetWindowSizeLimits(m_Window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
		glfwSetWindowMaximizeCallback(m_Window, _WindowMaximizeCallback);
		glfwSetTitlebarHitTestCallback(m_Window, _TitlebarHitTestCallback);

		
		// Center Window
		if (!m_Maximized) {
			GLFWmonitor* windowMonitor = glfwGetWindowMonitor(m_Window);
			if (!windowMonitor)
				windowMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* vidMode = glfwGetVideoMode(windowMonitor);
			int centeredPosX = (vidMode->width - m_Width) / 2;
			int centeredPosY = (vidMode->height - m_Height) / 2;
			glfwSetWindowPos(m_Window, centeredPosX, centeredPosY);
		}
	}

	Window::~Window() {
		SE_PROFILE_FUNCTION();

		s_WindowCount--;

		glfwDestroyWindow(m_Window);
		m_Window = nullptr;

		if (s_WindowCount == 0)
			glfwTerminate();
	}

	void Window::Update() {
		SE_PROFILE_FUNCTION();

		{
			SE_PROFILE_SCOPE("CallPendingCallbacks");

			for (auto& callback : m_PendingMainThreadCallbacks)
				callback();
			m_PendingMainThreadCallbacks.clear();
		}
		
		{
			SE_PROFILE_SCOPE("PollEvents");

			glfwPollEvents();
		}

		m_Minimized = glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
		m_Maximized = glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
		m_Hovered = glfwGetWindowAttrib(m_Window, GLFW_HOVERED);
		m_Focused = glfwGetWindowAttrib(m_Window, GLFW_FOCUSED);
		m_Fullscreen = glfwGetWindowMonitor(m_Window) != nullptr;
	}

	bool Window::ShouldClose() {
		return glfwWindowShouldClose(m_Window);
	}

	void Window::SetSize(uint32 width, uint32 height) {
		m_PendingMainThreadCallbacks.push_back([this, width, height]() {
			m_Width = width;
			m_Height = height;
			glfwSetWindowSize(m_Window, (int)m_Width, (int)m_Height);
		});
	}

	void Window::SetWidth(uint32 width) {
		m_PendingMainThreadCallbacks.push_back([this, width]() {
			m_Width = width;
			glfwSetWindowSize(m_Window, (int)m_Width, (int)m_Height);
		});
	}

	void Window::SetHeight(uint32 height) {
		m_PendingMainThreadCallbacks.push_back([this, height]() {
			m_Height = height;
			glfwSetWindowSize(m_Window, (int)m_Width, (int)m_Height);
		});
	}


	void Window::Maximize() {
		m_PendingMainThreadCallbacks.push_back([this]() {
			glfwMaximizeWindow(m_Window);
			m_Maximized = true;
			m_Minimized = false;
		});
	}

	void Window::Restore() {
		m_PendingMainThreadCallbacks.push_back([this]() {
			glfwRestoreWindow(m_Window);
			m_Maximized = false;
			m_Minimized = false;
		});
	}

	void Window::Minimize() {
		m_PendingMainThreadCallbacks.push_back([this]() {
			glfwIconifyWindow(m_Window);
			m_Maximized = false;
			m_Minimized = true;
		});
	}

	glm::ivec2 Window::GetFramebufferSize() {
		glm::ivec2 result;
		glfwGetFramebufferSize(m_Window, &result.x, &result.y);
		return result;
	}

	void Window::SetIcon(const std::filesystem::path& filepath) {
		if (!std::filesystem::exists(filepath)) {
			SE_ERR(Logging::Rendering, "IconFile '{}' doesn't exist!", filepath.string());
			return;
		}
		int w, h, channels;
		std::string filepathStr = filepath.string();
		stbi_set_flip_vertically_on_load(filepathStr.ends_with(".jpg"));
		unsigned char* pixels = stbi_load(filepathStr.c_str(), &w, &h, &channels, 4);
		GLFWimage image{ w, h, pixels };
		m_PendingMainThreadCallbacks.push_back([this, pixels, image]() {
			glfwSetWindowIcon(m_Window, 1, &image);
			stbi_image_free(pixels);
		});
	}

	void Window::_FramebufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window) {
			_window->m_Width = width;
			_window->m_Height = height;
			if (_window->m_EnableResizeCallback && _window->m_ResizeCallback)
				_window->m_ResizeCallback(_window);
		}
	}

	void Window::_WindowResizedCallback(GLFWwindow* window, int width, int height) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window) {
			_window->m_Width = width;
			_window->m_Height = height;
			if (_window->m_EnableResizeCallback && _window->m_ResizeCallback)
				_window->m_ResizeCallback(_window);
		}
	}

	void Window::_WindowPositionCallback(GLFWwindow* window, int xpos, int ypos) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window)
			_window->m_Position = { (float)xpos, (float)ypos };
	}

	void Window::_WindowMaximizeCallback(GLFWwindow* window, int maximized) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window)
			_window->m_Maximized = (bool)maximized;

		// Center Window position on restore
		if (maximized == 0) {
			GLFWmonitor* windowMonitor = glfwGetWindowMonitor(window);
			if (!windowMonitor)
				windowMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* vidMode = glfwGetVideoMode(windowMonitor);
			int windowWidth = 0, windowHeight = 0;
			glfwGetWindowSize(window, &windowWidth, &windowHeight);
			int centeredPosX = (vidMode->width - windowWidth) / 2;
			int centeredPosY = (vidMode->height - windowHeight) / 2;
			glfwSetWindowPos(window, centeredPosX, centeredPosY);
		}
	}

	void Window::_TitlebarHitTestCallback(GLFWwindow* window, [[maybe_unused]] int x, [[maybe_unused]] int y, int* hit) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window && _window->m_TitlebarhitTest)
			*hit = _window->m_TitlebarhitTest();
		else
			*hit = false;
	}
}