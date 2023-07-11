#include "pch.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"

namespace Sphynx::Rendering {
	VulkanContext::VulkanContext(Window& window)
		: m_Window(window)
	{
		m_Instance = std::make_unique<VulkanInstance>(
#if defined(DEBUG) || defined(RELEASE)
			true
#else
			false
#endif
		);

		m_Surface = window.GetSurface(m_Instance->Instance);

		std::vector<const char*> deviceExtensions {};
		m_PhysicalDevice = VulkanPhysicalDevice::Pick(m_Instance->Instance, m_Surface, deviceExtensions);
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);
		SE_INFO(Logging::Rendering, "Chosen GPU: {}", deviceProperties.deviceName);

		VulkanLogicalDevice::CreateResult logicalDeviceResult = VulkanLogicalDevice::Create(m_PhysicalDevice, m_Instance->ValidationLayers, m_Surface);
		m_LogicalDevice = logicalDeviceResult.Device;
		m_GraphicsQueue = logicalDeviceResult.GraphicsQueue;
		m_GraphicsQueue = logicalDeviceResult.PresentQueue;
		m_PresentQueue = logicalDeviceResult.PresentQueue;

	}

	VulkanContext::~VulkanContext() {
		vkDestroyDevice(m_LogicalDevice, nullptr);

		vkDestroySurfaceKHR(m_Instance->Instance, m_Surface, nullptr);

		m_Instance.reset();
	}
}