#include "pch.hpp"
#include "Window.hpp"

#include <GLFW/glfw3.h>

#include <stb_image.h>

namespace Sphynx::Rendering {
	void GLFWErrorCallback([[maybe_unused]] int error, [[maybe_unused]] const char* description) {
		SE_ERR(Logging::Rendering, "[GLFW]: ({0}): {1}", error, description);
	}

	Window::Window(const std::string_view title, uint32_t width, uint32_t height, bool fullscreen)
		: m_Title(title), m_Width(width), m_Height(height), m_Maximized(fullscreen)
	{
		SE_PROFILE_FUNCTION();

		if (s_WindowCount == 0) {
			SE_PROFILE_SCOPE("Sphynx::Rendering::Window::InitGLFW");
			SE_ASSERT(glfwInit(), Logging::Rendering, "Failed to initialize GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
		}
		s_WindowCount++;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_MAXIMIZED, m_Maximized);
		glfwWindowHint(GLFW_SAMPLES, 0);

		GLFWmonitor* monitor = nullptr;
		if (fullscreen) {
			monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
			m_Width = videoMode->width;
			m_Height = videoMode->height;
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

		glfwPollEvents();
	}

	bool Window::ShouldClose() {
		return glfwWindowShouldClose(m_Window);
	}

	void Window::SetSize(uint32_t width, uint32_t height) {
		m_Width = width;
		m_Height = height;
		glfwSetWindowSize(m_Window, (int)m_Width, (int)m_Height);
	}

	void Window::SetWidth(uint32_t width) {
		m_Width = width;
		glfwSetWindowSize(m_Window, (int)m_Width, (int)m_Height);
	}

	void Window::SetHeight(uint32_t height) {
		m_Height = height;
		glfwSetWindowSize(m_Window, (int)m_Width, (int)m_Height);
	}


	void Window::SetMaximized(bool maximized) {
		glfwSetWindowAttrib(m_Window, GLFW_MAXIMIZED, maximized);
	}

	bool Window::IsFocused() {
		return glfwGetWindowAttrib(m_Window, GLFW_FOCUSED);
	}

	bool Window::IsHovered() {
		return glfwGetWindowAttrib(m_Window, GLFW_HOVERED);
	}

	void Window::SetIcon(const std::filesystem::path& filepath) {
		std::string filepathStr = filepath.string();
		
		int w, h, channels;
		SE_ASSERT(std::filesystem::exists(filepath), Logging::Rendering, "IconFile '{}' doesn't exist!", filepathStr);
		stbi_set_flip_vertically_on_load(filepathStr.ends_with(".jpg"));
		unsigned char* pixels = stbi_load(filepathStr.c_str(), &w, &h, &channels, 4);
		GLFWimage image{ w, h, pixels };
		glfwSetWindowIcon(m_Window, 1, &image);
		stbi_image_free(pixels);
	}

	void Window::_FramebufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window) {
			if (_window->m_ResizeCallback)
				_window->m_ResizeCallback(_window);
			_window->m_Width = width;
			_window->m_Height = height;
		}
	}

	void Window::_WindowResizedCallback(GLFWwindow* window, int width, int height) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window) {
			if (_window->m_ResizeCallback)
				_window->m_ResizeCallback(_window);
			_window->m_Width = width;
			_window->m_Height = height;
		}
	}

	void Window::_WindowPositionCallback(GLFWwindow* window, int xpos, int ypos) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window)
			_window->m_Position = { (float)xpos, (float)ypos };
	}

	void Window::_WindowMaximizeCallback(GLFWwindow* window, int maximized) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (_window) {
			_window->m_Maximized = maximized == 1;
			_window->m_Minimized = maximized == 0;
		}
	}
}