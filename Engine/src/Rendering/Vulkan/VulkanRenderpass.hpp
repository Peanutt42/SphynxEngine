#pragma once

#include "Core/CoreInclude.hpp"
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
		VulkanRenderpass(RenderPassUsage usage, vk::Format format);
		~VulkanRenderpass();

		vk::RenderPass GetHandle() { return m_Renderpass; }

		// Isn't needed for the swapchain renderpass
		void CreateFramebuffers(uint32_t width, uint32_t height, vk::Format format);

		vk::Framebuffer GetFramebuffer(uint32_t currentImageIndex);

		void Begin(vk::Framebuffer framebuffer, vk::CommandBuffer commandBuffer, vk::Extent2D extent);
		void End(vk::CommandBuffer commandBuffer);

	private:
		vk::RenderPass m_Renderpass;

		// own framebuffers
		std::vector<vk::Framebuffer> m_Framebuffers;
		std::vector<vk::Image> m_FramebufferImages;
		std::vector<vk::DeviceMemory> m_FramebufferImageMemories;
		std::vector<vk::ImageView> m_FramebufferImageViews;
	};
}