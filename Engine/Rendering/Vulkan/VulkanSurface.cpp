#include "pch.hpp"

#ifdef WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR 1
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>
#include "VulkanSurface.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Sphynx::Rendering {
	vk::SurfaceKHR VulkanSurface::GetFromWindow(vk::Instance instance, GLFWwindow* window) {
		vk::Win32SurfaceCreateInfoKHR createInfo{};
		createInfo.hwnd = glfwGetWin32Window(window);
		createInfo.hinstance = GetModuleHandle(nullptr);
		vk::SurfaceKHR surface;
		vk::Result result = instance.createWin32SurfaceKHR(&createInfo, nullptr, &surface);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create window surface");
		return surface;
	}
}
#endif