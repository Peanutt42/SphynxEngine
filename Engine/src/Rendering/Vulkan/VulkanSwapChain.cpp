#include "pch.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "Core/Engine.hpp"

#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	VulkanSwapChain::VulkanSwapChain() {
		Create();
	}

	VulkanSwapChain::~VulkanSwapChain() {
		Cleanup();
	}

	void VulkanSwapChain::Recreate(VkRenderPass renderpass) {
		VulkanContext::Window->SetResizeCallbackEnable(false);
		while ((VulkanContext::Window->GetWidth() == 0 || VulkanContext::Window->GetHeight() == 0) && !Engine::ShouldClose()) {
			VulkanContext::Window->Update();
		}
		VulkanContext::Window->SetResizeCallbackEnable(true);
		if (Engine::ShouldClose())
			return;

		vkDeviceWaitIdle(VulkanContext::LogicalDevice);

		Cleanup();
		Create();
		CreateFramebuffers(renderpass);
	}

	void VulkanSwapChain::CreateFramebuffers(VkRenderPass renderpass) {
		m_Framebuffers.resize(m_ImageViews.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++) {
			VkImageView attachments[] = {
				m_ImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_Extent.width;
			framebufferInfo.height = m_Extent.height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(VulkanContext::LogicalDevice, &framebufferInfo, nullptr, &m_Framebuffers[i]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create framebuffer");
		}
	}

	VulkanSwapChain::SupportDetails VulkanSwapChain::GetSupport(VkPhysicalDevice device) {
		SupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VulkanContext::Surface, &details.Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanContext::Surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanContext::Surface, &formatCount, details.Formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanContext::Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanContext::Surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

	VkFramebuffer VulkanSwapChain::GetFramebuffer(uint32_t index) {
		if (index >= m_Framebuffers.size())
			return VK_NULL_HANDLE;
		return m_Framebuffers[index];
	}

	void VulkanSwapChain::Create() {
		VulkanContext::Window->Update();

		SupportDetails swapChainSupport = GetSupport(VulkanContext::PhysicalDevice);
		std::optional<VkSurfaceFormatKHR> surfaceFormat = ChooseFormat(swapChainSupport.Formats);
		SE_ASSERT(surfaceFormat.has_value(), Logging::Rendering, "Failed to get format for swapchain");
		VkPresentModeKHR presentMode = ChoosePresentMode(swapChainSupport.PresentModes);
		m_Extent = ChooseExtent(swapChainSupport.Capabilities, VulkanContext::Window->GetGLFWHandle());

		uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
			imageCount = swapChainSupport.Capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanContext::Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat->format;
		createInfo.imageColorSpace = surfaceFormat->colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VulkanQueueFamilyIndices indices(VulkanContext::PhysicalDevice);
		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(VulkanContext::LogicalDevice, &createInfo, nullptr, &m_SwapChain);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create swapchain");

		vkGetSwapchainImagesKHR(VulkanContext::LogicalDevice, m_SwapChain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		vkGetSwapchainImagesKHR(VulkanContext::LogicalDevice, m_SwapChain, &imageCount, m_Images.data());

		m_Format = surfaceFormat->format;

		m_ImageViews.resize(m_Images.size());
		for (size_t i = 0; i < m_ImageViews.size(); i++) {
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = m_Images[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = m_Format;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			VkResult imageViewResult = vkCreateImageView(VulkanContext::LogicalDevice, &imageViewCreateInfo, nullptr, &m_ImageViews[i]);
			SE_ASSERT(imageViewResult == VK_SUCCESS, Logging::Rendering, "Failed to create image view for swapchain");
		}
	}

	void VulkanSwapChain::Cleanup() {
		for (VkFramebuffer& framebuffer : m_Framebuffers) {
			vkDestroyFramebuffer(VulkanContext::LogicalDevice, framebuffer, nullptr);
			framebuffer = VK_NULL_HANDLE;
		}

		for (VkImageView& imageView : m_ImageViews) {
			vkDestroyImageView(VulkanContext::LogicalDevice, imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}

		vkDestroySwapchainKHR(VulkanContext::LogicalDevice, m_SwapChain, nullptr);
		m_SwapChain = VK_NULL_HANDLE;
	}

	std::optional<VkSurfaceFormatKHR> VulkanSwapChain::ChooseFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
		for (const auto& format : formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}
		return {};
	}

	VkPresentModeKHR VulkanSwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
		for (const auto& presentMode : presentModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return presentMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapChain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}