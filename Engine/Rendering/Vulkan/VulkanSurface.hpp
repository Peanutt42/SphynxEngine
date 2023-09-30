#pragma once

#include "pch.hpp"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Sphynx::Rendering {
	class VulkanSurface {
	public:
		static vk::SurfaceKHR GetFromWindow(vk::Instance instance, GLFWwindow* window);
	};
}