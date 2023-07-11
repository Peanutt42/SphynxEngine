#pragma once

#include "Core/CoreInclude.hpp"

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	class VulkanSwapChain {
	public:
		struct SupportDetails {
			VkSurfaceCapabilitiesKHR Capabilities{};
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;
		};

		static SupportDetails GetSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		VulkanSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, GLFWwindow* window);
		~VulkanSwapChain();


	private:
		static std::optional<VkSurfaceFormatKHR> ChooseFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	
		static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);

		static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
	
	private:
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		VkFormat m_Format;
		VkExtent2D m_Extent;
	};
}