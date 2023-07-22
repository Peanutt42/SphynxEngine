#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Window.hpp"

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	class VulkanSwapChain {
	public:
		struct SupportDetails {
			VkSurfaceCapabilitiesKHR Capabilities{};
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;
		};

		static SupportDetails GetSupport(VkPhysicalDevice device);

		VulkanSwapChain();
		~VulkanSwapChain();

		void Recreate(VkRenderPass renderpass);

		void CreateFramebuffers(VkRenderPass renderpass);

		const VkExtent2D& GetExtent() const { return m_Extent; }
		VkFormat GetFormat() const { return m_Format; }
		VkFramebuffer GetFramebuffer(uint32_t index);
		VkSwapchainKHR GetHandle() { return m_SwapChain; }

	private:
		void Create();
		void Cleanup();


		static std::optional<VkSurfaceFormatKHR> ChooseFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	
		static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);

		static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
	
	private:
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;
		VkFormat m_Format;
		VkExtent2D m_Extent;
	};
}