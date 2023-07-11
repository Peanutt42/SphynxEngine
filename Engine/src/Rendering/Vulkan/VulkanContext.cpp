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

		std::vector<const char*> deviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		m_PhysicalDevice = VulkanPhysicalDevice::Pick(m_Instance->Instance, m_Surface, deviceExtensions);
		
		SE_INFO(Logging::Rendering, "Chosen GPU: {}", VulkanPhysicalDevice::GetName(m_PhysicalDevice));

		VulkanLogicalDevice::CreateResult logicalDeviceResult = VulkanLogicalDevice::Create(m_PhysicalDevice, m_Instance->ValidationLayers, deviceExtensions, m_Surface);
		m_LogicalDevice = logicalDeviceResult.Device;
		m_GraphicsQueue = logicalDeviceResult.GraphicsQueue;
		m_GraphicsQueue = logicalDeviceResult.PresentQueue;
		m_PresentQueue = logicalDeviceResult.PresentQueue;

		m_SwapChain = std::make_unique<VulkanSwapChain>(m_PhysicalDevice, m_LogicalDevice, m_Surface, m_Window.GetGLFWHandle());
	}

	VulkanContext::~VulkanContext() {
		m_SwapChain.reset();

		vkDestroyDevice(m_LogicalDevice, nullptr);

		vkDestroySurfaceKHR(m_Instance->Instance, m_Surface, nullptr);

		m_Instance.reset();
	}
}