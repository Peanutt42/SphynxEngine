#pragma once

#include "Core/CoreInclude.hpp"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Sphynx::Rendering {
	class SE_API VulkanSurface {
	public:
		static vk::SurfaceKHR GetFromWindow(vk::Instance instance, GLFWwindow* window);
	};
}