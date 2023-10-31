#pragma once

#include "pch.hpp"
#include "Rendering/Window.hpp"

#include <vulkan/vulkan.hpp>

namespace Sphynx::Rendering {
	class VulkanSwapChain {
	public:
		struct SupportDetails {
			vk::SurfaceCapabilitiesKHR Capabilities{};
			std::vector<vk::SurfaceFormatKHR> Formats;
			std::vector<vk::PresentModeKHR> PresentModes;
		};

		static SupportDetails GetSupport(vk::PhysicalDevice device);

		VulkanSwapChain();
		~VulkanSwapChain();

		void Recreate(vk::RenderPass renderpass);

		void CreateFramebuffers(vk::RenderPass renderpass);

		const vk::Extent2D& GetExtent() const { return m_Extent; }
		vk::Format GetFormat() const { return m_Format; }
		vk::Framebuffer GetFramebuffer(uint32 index);
		vk::SwapchainKHR GetHandle() { return m_SwapChain; }

	private:
		VulkanSwapChain(const VulkanSwapChain&) = delete;
		VulkanSwapChain(VulkanSwapChain&&) = delete;
		VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
		VulkanSwapChain& operator=(VulkanSwapChain&&) = delete;

		void Create();

		static std::optional<vk::SurfaceFormatKHR> ChooseFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
	
		static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);

		static vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
	
	private:
		vk::SwapchainKHR m_SwapChain;

		std::vector<vk::Image> m_Images;
		std::vector<vk::ImageView> m_ImageViews;
		std::vector<vk::Framebuffer> m_Framebuffers;
		vk::Format m_Format = vk::Format::eUndefined;
		vk::Extent2D m_Extent;
	};
}