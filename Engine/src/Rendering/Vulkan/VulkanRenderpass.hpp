#pragma once

#include "pch.hpp"
#include "VulkanSwapChain.hpp"

namespace Sphynx::Rendering {
	enum class RenderPassUsage {
		Single,
		First,
		Middle,
		Last
	};

	class VulkanRenderpass {
	public:
		VulkanRenderpass(RenderPassUsage usage, VkFormat format);
		~VulkanRenderpass();

		VkRenderPass GetHandle() { return m_Renderpass; }

		// Isn't needed for the swapchain renderpass
		void CreateFramebuffers(uint32_t width, uint32_t height, VkFormat format);

		VkFramebuffer GetFramebuffer(uint32_t currentImageIndex);

		void Begin(VkFramebuffer framebuffer, VkCommandBuffer commandBuffer, VkExtent2D extent);
		void End(VkCommandBuffer commandBuffer);

	private:
		VkRenderPass m_Renderpass = VK_NULL_HANDLE;

		// own framebuffers
		std::vector<VkFramebuffer> m_Framebuffers;
		std::vector<VkImage> m_FramebufferImages;
		std::vector<VkDeviceMemory> m_FramebufferImageMemories;
		std::vector<VkImageView> m_FramebufferImageViews;
	};
}