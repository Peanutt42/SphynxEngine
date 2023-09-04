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
		VulkanRenderpass(RenderPassUsage usage, vk::Format format);
		~VulkanRenderpass();

		vk::RenderPass GetHandle() { return m_Renderpass; }

		// Isn't needed for the swapchain renderpass
		void CreateFramebuffers(uint32 width, uint32 height, vk::Format format, bool sampled);

		vk::Framebuffer GetFramebuffer(uint32 imageIndex);
		vk::Image GetImage(uint32 imageIndex);
		vk::ImageView GetImageView(uint32 imageIndex);
		const std::vector<vk::ImageView>& GetImageViews() { return m_FramebufferImageViews; }

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