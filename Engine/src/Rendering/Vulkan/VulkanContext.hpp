#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Window.hpp"
#include "VulkanInstance.hpp"

namespace Sphynx::Rendering {
	class VulkanContext {
	public:
		VulkanContext(Window& window);
		~VulkanContext();

	private:
		Window& m_Window;

		std::unique_ptr<VulkanInstance> m_Instance;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
	};
}