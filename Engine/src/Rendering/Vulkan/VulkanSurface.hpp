#pragma once

#include "Core/CoreInclude.hpp"

#undef VULKAN_H_
#ifdef WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR 1
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Sphynx::Rendering {
	class VulkanSurface {
	public:
		static VkSurfaceKHR GetFromWindow(VkInstance instance, GLFWwindow* window) {
#ifdef WINDOWS
			VkWin32SurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			createInfo.hwnd = glfwGetWin32Window(window);
			createInfo.hinstance = GetModuleHandle(nullptr);
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create window surface");
			return surface;
#endif
		}
	};
}